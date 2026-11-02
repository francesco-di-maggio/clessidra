// The web build: clessidra.wasm as the page's worklet drives it. From the
// repo: make test, after make web
import { readFileSync } from "node:fs";

const RATE = 48000;
const BLOCK = 128;
const KNOB = { S30: 0, S31: 1, S32: 2, S33: 3, S34: 4, S35: 5, S36: 6, S37: 7 };

let failed = 0;
function check(ok, what, value) {
  console.log(`${ok ? "ok  " : "FAIL"} ${what}${value === undefined ? "" : ` (${value})`}`);
  if (!ok) failed++;
}

const bytes = readFileSync(new URL("../build/web/clessidra.wasm", import.meta.url));
const { instance } = await WebAssembly.instantiate(bytes, {});
const w = instance.exports;
w._initialize?.();
const f32 = (ptr, n) => new Float32Array(w.memory.buffer, ptr, n);

check(w.cl_init(44100) === 0, "refuses a rate other than the board's");
check(w.cl_init(RATE) === 1, "starts at 48 kHz");

const io = [w.cl_input(0), w.cl_input(1), w.cl_output(0), w.cl_output(1)].map((p) => f32(p, BLOCK));
let now = 0;
let seed = 1;
const noise = () => ((seed = (seed * 1664525 + 1013904223) >>> 0) / 2 ** 31 - 1) * 0.25;

// Runs seconds of noise in, or silence; returns the output's peak and whether it stayed finite
function run(seconds, input = true) {
  let peak = 0;
  let finite = true;
  for (let b = 0; b < (seconds * RATE) / BLOCK; b++) {
    for (let i = 0; i < BLOCK; i++) io[0][i] = io[1][i] = input ? noise() : 0;
    w.cl_process(BLOCK, Math.floor(now));
    now += (BLOCK / RATE) * 1000;
    for (let i = 0; i < BLOCK; i++) {
      finite &&= Number.isFinite(io[2][i]) && Number.isFinite(io[3][i]);
      peak = Math.max(peak, Math.abs(io[2][i]), Math.abs(io[3][i]));
    }
  }
  return { peak, finite };
}

const view = () => f32(w.cl_view(), 18);
const grains = () => {
  const count = w.cl_grains();
  return Array.from({ length: count }, (_, g) => f32(w.cl_grain_fields() + g * 16, 4)[0]);
};

// Panel: speed 1, size at its centre, pitch unison, whole length, Pour in the middle
w.cl_knob(KNOB.S36, 0.75);
w.cl_knob(KNOB.S31, 0.5);
w.cl_knob(KNOB.S33, 0.5);
w.cl_knob(KNOB.S35, 1);
w.cl_knob(KNOB.S37, 0.9);

let r = run(1);
check(r.finite && r.peak === 0, "sealed and empty: silent", r.peak.toFixed(3));

w.cl_knob(KNOB.S34, 1);
r = run(3);
check(r.finite && r.peak > 0.01 && r.peak <= 1, "Fill up: grains of the input, under full scale", r.peak.toFixed(3));
let v = view();
check(v[0] > 0, "the write point moves", v[0]);
const peaks = f32(w.cl_peaks(), w.cl_columns());
check(peaks.some((p) => p > 0.1), "the waveform follows what is written");

// Every grain reads inside the window behind the write point
const live = grains();
const inLive = (x) => {
  const at = x * w.cl_buffer_samples();
  const from = v[5];
  const to = v[0];
  const wrap = w.cl_buffer_samples();
  return ((at - from + wrap) % wrap) <= ((to - from + wrap) % wrap) + 1;
};
check(live.length > 0 && live.every(inLive), "Fill 100 %: grains behind the write point", live.length);

w.cl_knob(KNOB.S34, 0);
r = run(1, false);
v = view();
check(Math.abs(v[2] - v[0]) < BLOCK, "sealed after 3 s: the sound ends at the write point", (v[2] / RATE).toFixed(2));
const edges = Array.from(v.slice(10, 18));
check(edges.every((e, i) => i === 0 || e >= edges[i - 1]), "slice edges in order", edges.map((e) => (e / RATE).toFixed(2)).join(" "));

// Held grains read inside the selection, or a grain past its end, with
// Scatter at 0 and Pour below the cloud, where the grains scatter by themselves
w.cl_knob(KNOB.S35, 0.25);
w.cl_knob(KNOB.S37, 0.6);
run(1, false);
v = view();
const wrap = w.cl_buffer_samples();
const held = grains();
const grainSamples = (f32(w.cl_params(), 13)[4] / 1000) * RATE;
const inSelection = (x) => ((x * wrap - v[3] + 2 * wrap) % wrap) <= v[4] + grainSamples;
check(held.length > 0 && held.every(inSelection), "sealed: grains inside the selection", held.length);

const params = f32(w.cl_params(), 13);
check(Math.abs(params[0] - 1) < 1e-6, "Tilt at three quarters: speed 1", params[0]);
check(params[6] === 0, "Fill at 0: sealed", params[6]);
check(w.cl_led(Math.floor(now)) >= 0, "LED in range");

if (failed) {
  console.log(`${failed} failed`);
  process.exit(1);
}
