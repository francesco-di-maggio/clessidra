// The firmware on the audio thread: clessidra.wasm (web/clessidra_web.cpp)
// runs here, one render quantum at a time. The page sends the controls and
// gets back what it draws, once per screen frame.

const FRAME = 1 / 60; // seconds between views
const VIEW = 18; // cl_view fields
const GRAIN_FIELDS = 4;
const PARAMS = 13;

class Clessidra extends AudioWorkletProcessor {
  constructor(options) {
    super();
    this.w = null;
    this.pending = [];
    this.next = 0;
    this.port.onmessage = (e) => (this.w ? this.apply(e.data) : this.pending.push(e.data));
    this.load(options.processorOptions.bytes);
  }

  async load(bytes) {
    const { instance } = await WebAssembly.instantiate(bytes, {});
    const w = instance.exports;
    w._initialize?.();
    if (!w.cl_init(sampleRate)) {
      this.port.postMessage({ type: "error", message: `The firmware runs at 48 kHz, this device at ${sampleRate / 1000} kHz.` });
      return;
    }
    this.f32 = (ptr, n) => new Float32Array(w.memory.buffer, ptr, n);
    this.io = [w.cl_input(0), w.cl_input(1), w.cl_output(0), w.cl_output(1)].map((p) => this.f32(p, 128));
    this.peaks = this.f32(w.cl_peaks(), w.cl_columns());
    this.w = w;
    for (const m of this.pending) this.apply(m);
    this.pending = null;
    this.port.postMessage({ type: "ready", columns: w.cl_columns(), samples: w.cl_buffer_samples() });
  }

  apply(m) {
    if (m.type === "knob") this.w.cl_knob(m.index, m.value);
    else if (m.type === "switch") this.w.cl_switch(m.index, m.position);
    else if (m.type === "pad") this.w.cl_pad(m.index, m.touched ? 1 : 0, m.pressure);
  }

  process(inputs, outputs) {
    const out = outputs[0];
    if (!this.w) return true;
    const n = out[0].length;
    const [inL, inR = inL] = inputs[0] ?? [];
    if (inL) {
      this.io[0].set(inL);
      this.io[1].set(inR);
    } else {
      this.io[0].fill(0);
      this.io[1].fill(0);
    }
    const now = Math.floor(currentTime * 1000);
    this.w.cl_process(n, now);
    out[0].set(this.io[2].subarray(0, n));
    out[1]?.set(this.io[3].subarray(0, n));
    if (currentTime >= this.next) {
      this.next = currentTime + FRAME;
      this.send(now);
    }
    return true;
  }

  send(now) {
    const w = this.w;
    const count = w.cl_grains();
    const view = this.f32(w.cl_view(), VIEW).slice();
    const grains = this.f32(w.cl_grain_fields(), count * GRAIN_FIELDS).slice();
    const params = this.f32(w.cl_params(), PARAMS).slice();
    const peaks = this.peaks.slice();
    this.port.postMessage({ type: "view", led: w.cl_led(now), view, grains, params, peaks }, [view.buffer, grains.buffer, params.buffer, peaks.buffer]);
  }
}

registerProcessor("clessidra", Clessidra);
