// The firmware on the audio thread: clessidra.wasm (web/clessidra_web.cpp)
// runs here, one render quantum at a time. The page sends the controls and
// gets back what it draws, once per screen frame.

import { BLOCK, GRAIN_FIELDS, INCOMING_COLUMNS, INCOMING_SECONDS, PARAM_SIZE, SLICES, VIEW_SIZE, type Control, type Exports, type FromWorklet } from "./engine";

// The AudioWorkletGlobalScope, as far as this uses it
declare const sampleRate: number;
declare const currentTime: number;
declare class AudioWorkletProcessor {
  readonly port: MessagePort;
  constructor(options?: { processorOptions?: unknown });
}
declare function registerProcessor(name: string, processor: new (options: { processorOptions: { bytes: ArrayBuffer } }) => AudioWorkletProcessor): void;

const FRAME_S = 1 / 60;

class Clessidra extends AudioWorkletProcessor {
  private w: Exports | null = null;
  private pending: Control[] = [];
  private next = 0;
  private io: Float32Array[] = [];
  private peaks = new Float32Array(0);
  private input = 0; // peaks since the last frame
  private output = 0;
  // The input's peak per column, a ring of the last INCOMING_SECONDS
  private incoming = new Float32Array(INCOMING_COLUMNS);
  private column = 0;
  private columnPeak = 0;
  private columnSamples = 0;

  constructor(options: { processorOptions: { bytes: ArrayBuffer } }) {
    super();
    this.port.onmessage = (e: MessageEvent<Control>) => (this.w ? this.apply(e.data) : this.pending.push(e.data));
    this.load(options.processorOptions.bytes);
  }

  private f32(ptr: number, n: number) {
    return new Float32Array(this.w!.memory.buffer, ptr, n);
  }

  private post(m: FromWorklet, transfer: Transferable[] = []) {
    this.port.postMessage(m, transfer);
  }

  private async load(bytes: ArrayBuffer) {
    const { instance } = await WebAssembly.instantiate(bytes, {});
    const w = instance.exports as unknown as Exports;
    w._initialize?.();
    if (!w.cl_init(sampleRate)) {
      this.post({ type: "error", message: `The firmware runs at 48 kHz, this device at ${sampleRate / 1000} kHz.` });
      return;
    }
    this.w = w;
    this.io = [w.cl_input(0), w.cl_input(1), w.cl_output(0), w.cl_output(1)].map((p) => this.f32(p, BLOCK));
    this.peaks = this.f32(w.cl_peaks(), w.cl_columns());
    for (const m of this.pending) this.apply(m);
    this.pending = [];
    this.post({ type: "ready" });
  }

  private apply(m: Control) {
    const w = this.w!;
    if (m.type === "knob") w.cl_knob(m.index, m.value);
    else if (m.type === "tap") w.cl_tap_seconds(m.seconds);
    else if (m.type === "switch") w.cl_switch(m.index, m.position);
    else w.cl_pad(m.index, m.touched ? 1 : 0, m.pressure);
  }

  process(inputs: Float32Array[][], outputs: Float32Array[][]) {
    const w = this.w;
    if (!w) return true;
    const out = outputs[0];
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
    w.cl_process(n, now);
    out[0].set(this.io[2].subarray(0, n));
    out[1]?.set(this.io[3].subarray(0, n));
    const perColumn = (INCOMING_SECONDS * sampleRate) / INCOMING_COLUMNS;
    for (let i = 0; i < n; i++) {
      const x = Math.max(Math.abs(this.io[0][i]), Math.abs(this.io[1][i]));
      this.input = Math.max(this.input, x);
      this.output = Math.max(this.output, Math.abs(this.io[2][i]), Math.abs(this.io[3][i]));
      this.columnPeak = Math.max(this.columnPeak, x);
      if (++this.columnSamples >= perColumn) {
        this.incoming[this.column] = this.columnPeak;
        this.column = (this.column + 1) % INCOMING_COLUMNS;
        this.columnPeak = this.columnSamples = 0;
      }
    }
    if (currentTime >= this.next) {
      this.next = currentTime + FRAME_S;
      this.send(now);
    }
    return true;
  }

  private send(now: number) {
    const w = this.w!;
    const count = w.cl_grains();
    const view = this.f32(w.cl_view(), VIEW_SIZE).slice();
    const grains = this.f32(w.cl_grain_fields(), count * GRAIN_FIELDS).slice();
    const params = this.f32(w.cl_params(), PARAM_SIZE).slice();
    const peaks = this.peaks.slice();
    const slices = this.f32(w.cl_slices(), SLICES).slice();
    const incoming = new Float32Array(INCOMING_COLUMNS);
    incoming.set(this.incoming.subarray(this.column));
    incoming.set(this.incoming.subarray(0, this.column), INCOMING_COLUMNS - this.column);
    this.post({ type: "view", led: w.cl_led(now), input: this.input, output: this.output, incoming, view, grains, params, peaks, slices }, [view.buffer, grains.buffer, params.buffer, peaks.buffer, incoming.buffer, slices.buffer]);
    this.input = this.output = 0;
  }
}

registerProcessor("clessidra", Clessidra);
