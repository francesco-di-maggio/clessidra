// The web build: clessidra.wasm as the page's worklet drives it. From the
// repo: make test, after make web
import { readFileSync } from "node:fs";

const RATE = 48000;
const BLOCK = 128;
const KNOB = { S30: 0, S31: 1, S32: 2, S33: 3, S34: 4, S35: 5, S36: 6, S37: 7 };
const SPEED_1 = 0.63; // S36 on its fourth stop from the bottom: 1×
const RECORD = 10; // CoreUI::kRecordPad
const V = { sound: 0, selStart: 1, selLength: 2, playhead: 3, dropped: 4 };

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
// Plucks: a decaying tone every half second
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

const view = () => f32(w.cl_view(), 5);
const grains = () => Array.from({ length: w.cl_grains() }, (_, g) => f32(w.cl_grain_fields() + g * 24, 6)[0]);
const silence = () => 0;

// Panel: speed 1, 100 ms, pitch unison, Hann, whole selection, plain grains,
// random, about one at once, looped
w.cl_knob(KNOB.S36, SPEED_1);
w.cl_knob(KNOB.S31, 0.5);
w.cl_knob(KNOB.S32, 0);
w.cl_knob(KNOB.S33, 0.5);
w.cl_knob(KNOB.S34, 0.5);
w.cl_knob(KNOB.S30, 0);
w.cl_knob(KNOB.S35, 1);
w.cl_knob(KNOB.S37, 0.75);
w.cl_switch(0, 1);
w.cl_switch(1, 2);

let r = run(3);
check(r.finite && r.peak === 0, "nothing recorded: silent", r.peak.toFixed(3));

// Hold P10 for 2 s
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

// A tap keeps the last 8 s
run(9);
w.cl_pad(RECORD, 1, 0.5);
run(0.1);
w.cl_pad(RECORD, 0, 0);
run(0.5);
v = view();
check(w.cl_recordings() === 2 && Math.abs(v[V.sound] / RATE - 8) < 0.02, "tap: the last 8 s", (v[V.sound] / RATE).toFixed(2));

// S35 at half, S30 at the top: the second half. Grains inside, with no
// randomness, many at once
w.cl_knob(KNOB.S30, 1);
w.cl_knob(KNOB.S35, 0.5);
w.cl_knob(KNOB.S37, 1);
run(1, silence);
v = view();
check(Math.abs(v[V.selStart] / v[V.sound] - 0.5) < 1e-3 && Math.abs(v[V.selLength] / v[V.sound] - 0.5) < 1e-3, "S35 half, S30 top: the second half");
const grainSamples = (f32(w.cl_params(), 13)[3] / 1000) * RATE;
const held = grains();
const inside = held.every((x) => {
  const at = x * v[V.sound];
  return at >= v[V.selStart] - 1 && at <= v[V.selStart] + v[V.selLength] + grainSamples;
});
check(held.length > 0 && inside, "grains inside the selection", held.length);
w.cl_knob(KNOB.S37, 0.75);

// S35 at a quarter, S30 at the middle: the middle quarter, the playhead
// inside it
w.cl_knob(KNOB.S35, 0.25);
w.cl_knob(KNOB.S30, 0.5);
w.cl_knob(KNOB.S36, 1);
let inLoop = true;
for (let i = 0; i < 40; i++) {
  run(0.05, silence);
  const at = view()[V.playhead] / v[V.sound];
  inLoop &&= at >= 0.375 - 1e-3 && at < 0.625 + 1e-3;
}
check(inLoop, "S35 quarter, S30 middle: the middle quarter, the playhead inside it");
w.cl_knob(KNOB.S30, 0.5);
w.cl_knob(KNOB.S36, SPEED_1);
run(0.1, silence);

const params = f32(w.cl_params(), 13);
check(Math.abs(params[0] - 1) < 1e-6, "Speed on its fourth stop: 1", params[0]);

// P11 held: S37 sets the reverb, Density stays as it was
const SHIFT = 11;
const reverb = () => f32(w.cl_params(), 22)[5];
const density = () => f32(w.cl_params(), 22)[2];
const before = density();
w.cl_pad(SHIFT, 1, 0.5);
run(0.02, silence);
w.cl_knob(KNOB.S37, 1);
run(0.02, silence);
const shifted = reverb();
w.cl_pad(SHIFT, 0, 0);
run(0.02, silence);
check(shifted > 0.9 && density() === before, "P11 + S37: the reverb, Density kept", shifted.toFixed(2));
w.cl_knob(KNOB.S37, 0.75);
run(0.02, silence);

// S37 at the centre: no grains. At the top: a cloud, 16 at once
w.cl_knob(KNOB.S37, 0.5);
run(1, silence);
check(w.cl_grains() === 0, "S37 centre: no grains", w.cl_grains());
w.cl_knob(KNOB.S37, 1);
run(1, silence);
let sounding = 0;
for (let i = 0; i < 50; i++) {
  run(0.02, silence);
  sounding += w.cl_grains() / 50;
}
check(Math.abs(sounding - 16) < 3, "S37 top: a cloud, 16 at once", sounding.toFixed(1));
w.cl_knob(KNOB.S37, 0.75);
check(w.cl_led(Math.floor(now)) >= 0, "LED in range");

if (failed) {
  console.log(`${failed} failed`);
  process.exit(1);
}
