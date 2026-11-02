// The Simple Touch's controls in this firmware: where each sits on the board,
// what it does, and how its value reads. Indices follow Controls
// (src/hw/controls.h). Positions are the board's, in the 600 × 973 photo's
// pixels (touch.jpeg). Play, Mapping and the panel drawing read them.

import { LAYER_PAD, PARAM, RATE, RECORD_PAD, SLICE_PADS } from "./engine";

export const POSITION = { centre: 0, up: 1, down: 2 } as const; // libDaisy's Switch3
export const END = ["Off", "Ping-pong", "On"]; // Playhead::End, by S07
export const STEPS = ["Free", "5th", "Oct"]; // Core::Steps

// What a readout knows besides the parameters
export interface Live {
  sound: number; // samples recorded
  slice: number; // playing, NUM_SLICES for the whole selection
  recording: number; // seconds P11 has been held, 0 when not
}

export interface Control {
  id: string;
  type: "knob" | "fader" | "switch" | "pad";
  index: number; // Controls' knob, switch or pad
  name: string;
  x: number;
  y: number;
  initial?: number;
  bipolar?: boolean; // drawn from its centre
  slice?: number;
  free?: boolean;
  second?: boolean; // the knob's function while P11 is held
  board?: string; // where it is on the board, if not its id
  does?: string;
  value?: (p: Float32Array, v: Live) => string;
}

const seconds = (samples: number) => `${(samples / RATE).toFixed(samples < RATE ? 2 : 1)} s`;
const percent = (x: number) => `${Math.round(x * 100)} %`;
const signed = (x: number, digits: number) => `${x > 0 ? "+" : x < 0 ? "−" : ""}${Math.abs(x).toFixed(digits)}`;
const span = (p: Float32Array, v: Live) => `${seconds(p[PARAM.start] * v.sound)} – ${seconds((p[PARAM.start] + p[PARAM.length]) * v.sound)}`;

const PAD_AT: Record<number, [number, number]> = {
  0: [150, 620], 1: [310, 610], 2: [412, 625], 3: [48, 800], 4: [170, 765], 5: [285, 795],
  6: [420, 788], 7: [555, 835], 8: [190, 915], 9: [405, 920], 10: [240, 50], 11: [370, 50],
};
const padAt = (index: number) => ({ x: PAD_AT[index][0], y: PAD_AT[index][1] });

export const CONTROLS: Control[] = [
  {
    id: "S36", type: "fader", index: 6, x: 36, y: 0, name: "Speed", initial: 0.75, bipolar: true,
    does: "Centre stops. Up forward, down backward, up to 4×. 1× half way.",
    value: (p) => (p[PARAM.freeze] ? "Stopped" : `${p[PARAM.speed] > 0 ? "→" : "←"} ${Math.abs(p[PARAM.speed]).toFixed(2)}×`),
  },
  {
    id: "S37", type: "fader", index: 7, x: 564, y: 0, name: "Density", initial: 0.6,
    does: "Grains per second, 0.3 to 131, off at the bottom. A firm press on a slice raises it.",
    value: (p) => (p[PARAM.density] === 0 ? "Closed" : `${p[PARAM.density].toFixed(p[PARAM.density] < 10 ? 1 : 0)}/s`),
  },
  {
    id: "S30", type: "knob", index: 0, x: 120, y: 478, name: "Start", initial: 0,
    does: "One end of the selection. The playhead moves between Start and End, in either order.",
    value: span,
  },
  {
    id: "S35", type: "knob", index: 5, x: 476, y: 478, name: "End", initial: 1,
    does: "The other end of the selection.",
    value: span,
  },
  {
    id: "S31", type: "knob", index: 1, x: 120, y: 350, name: "Size", initial: 0.5, bipolar: true,
    does: "Grain length, 10 ms to 4 s, at most half the recording. 200 ms at the centre.",
    value: (p) => `${Math.round(p[PARAM.grainMs])} ms`,
  },
  {
    id: "S32", type: "knob", index: 2, x: 240, y: 350, name: "Scatter", initial: 0,
    does: "Randomness, grain by grain: start times, position (up to ±2 s), size (½ to 2×) and pan.",
    value: (p) => percent(p[PARAM.jitter]),
  },
  {
    id: "S33", type: "knob", index: 3, x: 358, y: 350, name: "Pitch", initial: 0.5, bipolar: true,
    does: "±2 octaves, stops at 0, 0.1, 1, 3, 4, 7, 12 and 24 semitones.",
    value: (p) => `${signed(p[PARAM.pitch], Math.abs(p[PARAM.pitch]) < 1 && p[PARAM.pitch] !== 0 ? 1 : 0)} st`,
  },
  {
    id: "S34", type: "knob", index: 4, x: 476, y: 350, name: "Dry/Wet", initial: 0.5,
    does: "The input and the grains, equal power. 0 % dry, 100 % wet.",
    value: (p) => percent(p[PARAM.mix]),
  },
  {
    id: "P10", type: "pad", index: RECORD_PAD, ...padAt(RECORD_PAD), name: "Record",
    does: "Tap: keeps the last 8 s. Hold: records until you let go, up to 30 s. The glass always listens.",
    value: (_, v) => (v.recording ? `● ${v.recording.toFixed(1)} s` : v.sound ? `${seconds(v.sound)} held` : "Empty"),
  },
  {
    id: "S07", type: "switch", index: 1, x: 330, y: 690, name: "Loop", initial: POSITION.down,
    does: "At the end of the selection the playhead stops (off, up), turns (ping-pong, centre) or wraps (on, down).",
    value: (p) => END[p[PARAM.end]],
  },
  ...SLICE_PADS.map((pad, i): Control => ({
    id: `P${String(pad).padStart(2, "0")}`, type: "pad", index: pad, ...padAt(pad), name: `Slice ${i + 1}`, slice: i,
    does: "Slices at the sound's onsets. A touch plays one, a firm press raises Density.",
    value: (_, v) => (v.slice === i ? "Playing" : ""),
  })),
  {
    id: "S09", type: "switch", index: 0, x: 222, y: 638, name: "Steps", initial: POSITION.up,
    does: "The steps of pitch scatter: octaves (up), fifths and octaves (centre), free (down).",
    value: (p) => STEPS[p[PARAM.steps]],
  },
  // While P11 is held, three knobs do something else
  {
    id: "P11+S31", type: "knob", index: 1, x: 0, y: 0, name: "Shape", initial: 0.5, bipolar: true, second: true, board: "P11 + S31",
    does: "The grain window: reverse percussive, Hann at the centre, percussive.",
    value: (p) => (Math.abs(p[PARAM.shape] - 0.5) < 0.01 ? "Hann" : `${p[PARAM.shape] > 0.5 ? "Perc" : "Rev"} ${percent(Math.abs(p[PARAM.shape] - 0.5) * 2)}`),
  },
  {
    id: "P11+S33", type: "knob", index: 3, x: 0, y: 0, name: "Scatter", initial: 0, second: true, board: "P11 + S33",
    does: "Pitch scatter: most grains stay near the pitch, a few go up to ±2 octaves, on the steps S09 sets.",
    value: (p) => percent(Math.cbrt(p[PARAM.randPitch])),
  },
  {
    id: "P11+S34", type: "knob", index: 4, x: 0, y: 0, name: "Reverb", initial: 0.25, second: true, board: "P11 + S34",
    does: "Reverb on the grains: more and longer, 0.5 to 16 s.",
    value: (p) => percent(p[PARAM.reverb]),
  },
  { id: "P11", type: "pad", index: LAYER_PAD, ...padAt(LAYER_PAD), name: "Shift", does: "Held: the second layer. S31 shape, S33 pitch scatter, S34 reverb. Each knob takes over once it reaches the value." },
  ...[0, 1, 2].map((pad): Control => ({ id: `P${String(pad).padStart(2, "0")}`, type: "pad", index: pad, ...padAt(pad), name: "Free", free: true })),
];

export const byId = (id: string) => CONTROLS.find((c) => c.id === id)!;
export const padOf = (index: number) => CONTROLS.find((c) => c.type === "pad" && c.index === index)!;

export const LED: [string, string][] = [
  ["Glow", "Grains playing"],
  ["Flashing", "Recording"],
  ["Pulse", "A control at or turning within its centre: pitch unison, speed stopped or 1×, size at its centre"],
  ["Steady", "P11 held: the value of the knob last turned"],
];

// Browser only: the level into the firmware and out of it. The board has its
// own, on the hardware.
export interface Level extends Control {
  gain: (x: number) => number; // linear
}

const db = (gain: number) => (gain < 0.001 ? "−∞ dB" : `${signed(20 * Math.log10(gain), 1)} dB`);

export const LEVELS: Level[] = [
  {
    id: "IN", type: "knob", index: -1, x: 0, y: 0, name: "In", initial: 0.5, bipolar: true,
    does: "The level into the firmware, ±12 dB. Browser only: on the board, the source sets it.",
    gain: (x) => 10 ** (((x - 0.5) * 24) / 20),
  },
  {
    id: "OUT", type: "knob", index: -1, x: 0, y: 0, name: "Out", initial: 1,
    does: "The volume of what you hear, −60 to 0 dB, off at the bottom. Browser only.",
    gain: (x) => (x <= 0 ? 0 : 10 ** ((60 * (x - 1)) / 20)),
  },
];

// The levels' knob positions, as the page last set them
export const LEVELS_VALUE = new Map<string, number>();
for (const l of LEVELS) l.value = () => db(l.gain(LEVELS_VALUE.get(l.id) ?? l.initial ?? 0));
