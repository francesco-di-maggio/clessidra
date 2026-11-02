// The firmware on the page, silent, until Start: it computes what the knobs
// read, so every value is the firmware's own from the first paint.

import { BLOCK, PARAM_SIZE, RATE, VIEW_SIZE, type Control, type Exports } from "./engine";
import { url } from "./url";

export class Preview {
  private constructor(private w: Exports) {}

  static async load() {
    const bytes = await (await fetch(url("clessidra.wasm").replace(/\/$/, ""))).arrayBuffer();
    const { instance } = await WebAssembly.instantiate(bytes, {});
    const w = instance.exports as unknown as Exports;
    w._initialize?.();
    w.cl_init(RATE);
    return new Preview(w);
  }

  apply(m: Control) {
    const w = this.w;
    if (m.type === "knob") w.cl_knob(m.index, m.value);
    else if (m.type === "second") w.cl_second(m.index, m.value);
    else if (m.type === "switch") w.cl_switch(m.index, m.position);
  }

  // One silent block, then the parameters and the view it leaves
  read() {
    const w = this.w;
    w.cl_process(BLOCK, 0);
    const f32 = (ptr: number, n: number) => new Float32Array(w.memory.buffer, ptr, n).slice();
    return { params: f32(w.cl_params(), PARAM_SIZE), view: f32(w.cl_view(), VIEW_SIZE) };
  }
}
