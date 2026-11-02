// The web build: clessidra.wasm as the page's worklet drives it. From the
// repo: make test, after make web
import { readFileSync } from "node:fs";

const RATE = 48000;
const BLOCK = 128;
const KNOB = { S30: 0, S31: 1, S32: 2, S33: 3, S34: 4, S35: 5, S36: 6, S37: 7 };
const RECORD = 11;
const V = { sound: 0, selStart: 1, selLength: 2, playhead: 3, slice: 4, edges: 5 };

let failed = 0;
function check(ok, what, value) {
  console.log(`${ok ? "ok  " : "FAIL"} ${what}${value === undefined ? "" : ` (${value})`}`);
  if (!ok) failed++;
}

const bytes = readFileSync(new URL("../web/public/clessidra.wasm", import.meta.url));
const { instance } = await WebAssembly.instantiate(bytes, {});
const w = instance.exports;
w._initialize?.();
const f32 = (ptr, n) => new Float32Array(w.memory.buffer, ptr, n);

check(w.cl_init(44100) === 0, "refuses a rate other than the board's");
check(w.cl_init(RATE) === 1, "starts at 48 kHz");

const io = [w.cl_input(0), w.cl_input(1), w.cl_output(0), w.cl_output(1)].map((p) => f32(p, BLOCK));
let now = 0;
let t = 0;
// Plucks: a decaying tone every half second, so the slices find onsets
const pluck = () => {
  const s = (t++ % (RATE / 2)) / RATE;
  return 0.4 * Math.exp(-8 * s) * Math.sin(2 * Math.PI * 330 * s);
};

// Runs seconds of input; returns the output's peak and whether it stayed finite
function run(seconds, input = pluck) {
  let peak = 0;
  let finite = true;
  for (let b = 0; b < (seconds * RATE) / BLOCK; b++) {
    for (let i = 0; i < BLOCK; i++) io[0][i] = io[1][i] = input();
    w.cl_process(BLOCK, Math.floor(now));
    now += (BLOCK / RATE) * 1000;
    for (let i = 0; i < BLOCK; i++) {
      finite &&= Number.isFinite(io[2][i]) && Number.isFinite(io[3][i]);
      peak = Math.max(peak, Math.abs(io[2][i]), Math.abs(io[3][i]));
    }
  }
  return { peak, finite };
}

const view = () => f32(w.cl_view(), 13);
const grains = () => Array.from({ length: w.cl_grains() }, (_, g) => f32(w.cl_grain_fields() + g * 16, 4)[0]);
const silence = () => 0;

// Panel: speed 1, size at its centre, pitch unison, whole selection, Pour in
// the middle
w.cl_knob(KNOB.S36, 0.75);
w.cl_knob(KNOB.S31, 0.5);
w.cl_knob(KNOB.S33, 0.5);
w.cl_knob(KNOB.S30, 0);
w.cl_knob(KNOB.S35, 1);
w.cl_knob(KNOB.S37, 0.6);
w.cl_knob(KNOB.S34, 1);

let r = run(3);
check(r.finite && r.peak === 0, "nothing recorded: silent", r.peak.toFixed(3));

// Hold P11 for 2 s
w.cl_pad(RECORD, 1, 0.5);
run(2);
w.cl_pad(RECORD, 0, 0);
run(0.5);
let v = view();
check(w.cl_recordings() === 1 && Math.abs(v[V.sound] / RATE - 2.05) < 0.02, "hold 2 s: the touch and 50 ms before", (v[V.sound] / RATE).toFixed(2));
r = run(2, silence);
check(r.finite && r.peak > 0.01 && r.peak <= 1, "the recording plays, under full scale", r.peak.toFixed(3));
const peaks = f32(w.cl_peaks(), w.cl_columns());
check(peaks.some((p) => p > 0.1), "the waveform shows the recording");
const edges = Array.from(view().slice(V.edges, V.edges + 8));
check(edges.every((e, i) => i === 0 || e > edges[i - 1]), "slices at the onsets, in order", edges.map((e) => (e / RATE).toFixed(2)).join(" "));

// A tap keeps the last 8 s
run(9);
w.cl_pad(RECORD, 1, 0.5);
run(0.1);
w.cl_pad(RECORD, 0, 0);
run(0.5);
v = view();
check(w.cl_recordings() === 2 && Math.abs(v[V.sound] / RATE - 8) < 0.02, "tap: the last 8 s", (v[V.sound] / RATE).toFixed(2));

// S30 above S35: the selection between them. Grains inside, with Scatter at 0
// and Pour below the cloud, where the grains scatter by themselves
w.cl_knob(KNOB.S30, 0.75);
w.cl_knob(KNOB.S35, 0.25);
run(1, silence);
v = view();
check(Math.abs(v[V.selStart] / v[V.sound] - 0.25) < 1e-3 && Math.abs(v[V.selLength] / v[V.sound] - 0.5) < 1e-3, "S30 above S35: the selection between them");
const grainSamples = (f32(w.cl_params(), 14)[4] / 1000) * RATE;
const held = grains();
const inside = held.every((x) => {
  const at = x * v[V.sound];
  return at >= v[V.selStart] - 1 && at <= v[V.selStart] + v[V.selLength] + grainSamples;
});
check(held.length > 0 && inside, "grains inside the selection", held.length);

const params = f32(w.cl_params(), 14);
check(Math.abs(params[0] - 1) < 1e-6, "Tilt at three quarters: speed 1", params[0]);
check(w.cl_led(Math.floor(now)) >= 0, "LED in range");

if (failed) {
  console.log(`${failed} failed`);
  process.exit(1);
}
