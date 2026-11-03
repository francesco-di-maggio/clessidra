// The Simple Touch's controls in this firmware: where each sits on the board,
// what it does, and how its value reads. Indices follow Controls
// (src/hw/controls.h). Positions are the board's, in the 600 × 973 photo's
// pixels (touch.jpeg). Play, Mapping and the panel drawing read them.

import { LAYER, MUTE_PAD, PARAM, RATE, RECORD_PAD, RESTART_PAD, SHIFT_PAD, SLICE_PADS } from "./engine";

export const POSITION = { centre: 0, up: 1, down: 2 } as const; // libDaisy's Switch3

// What a readout knows besides the parameters
export interface Live {
  sound: number; // samples recorded
  triggers: boolean[]; // each slice pad's trigger sounding
  slices: number[]; // each slice's length, samples
  recording: number; // seconds P10 has been held, 0 when not
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
  detent?: boolean; // stops at its centre
  slice?: number;
  free?: boolean;
  layer?: number; // the knob's function while a pad holds this layer (LAYER)
  short?: string; // its label under its knob, where the name repeats it
  board?: string; // where it is on the board, if not its id
  does?: string;
  value?: (p: Float32Array, v: Live) => string;
}

const seconds = (samples: number) => `${(samples / RATE).toFixed(samples < RATE ? 2 : 1)} s`;
const percent = (x: number) => `${Math.round(x * 100)} %`;
const ms = (x: number) => (x < 1000 ? `${Math.round(x)} ms` : `${(x / 1000).toFixed(2)} s`);
const signed = (x: number, digits: number) => `${x > 0 ? "+" : x < 0 ? "−" : ""}${Math.abs(x).toFixed(digits)}`;
// A randomness, -1..1: up an even spread, down mostly near with a long tail
const random = (x: number) => (x === 0 ? "Off" : `${signed(x * 100, 0)} %`);

const PAD_AT: Record<number, [number, number]> = {
  0: [150, 620], 1: [310, 610], 2: [412, 625], 3: [48, 800], 4: [170, 765], 5: [285, 795],
  6: [420, 788], 7: [555, 835], 8: [190, 915], 9: [405, 920], 10: [240, 50], 11: [370, 50],
};
const padAt = (index: number) => ({ x: PAD_AT[index][0], y: PAD_AT[index][1] });

export const CONTROLS: Control[] = [
  {
    id: "S36", type: "fader", index: 6, x: 36, y: 0, name: "Speed", initial: 0.5, detent: true,
    does: "Playhead speed, 0 to 4×. 1× at the centre, stopped at the bottom.",
    value: (p) => (p[PARAM.freeze] ? "Frozen" : `${Math.abs(p[PARAM.speed]).toFixed(2)}×`),
  },
  {
    id: "S37", type: "fader", index: 7, x: 564, y: 0, name: "Filter", initial: 0.5, bipolar: true, detent: true,
    does: "Low-pass down, 20 kHz to 200 Hz. Off at the centre. High-pass up, to 2 kHz.",
    value: (p) => {
      const t = p[PARAM.filter];
      if (t === 0) return "Off";
      const hz = t < 0 ? 20000 * (200 / 20000) ** -t : 20 * (2000 / 20) ** t;
      return `${t < 0 ? "LP" : "HP"} ${hz < 1000 ? Math.round(hz) : (hz / 1000).toFixed(1) + "k"}`;
    },
  },
  {
    id: "S30", type: "knob", index: 0, x: 120, y: 478, name: "Position", initial: 0,
    does: "Selection start, anywhere in the recording.",
    value: (p, v) => seconds(p[PARAM.start] * v.sound),
  },
  {
    id: "S35", type: "knob", index: 5, x: 476, y: 478, name: "Length", initial: 1,
    does: "Selection length, past the end round to the start. All at the top.",
    value: (p, v) => seconds(p[PARAM.length] * v.sound),
  },
  {
    id: "S31", type: "knob", index: 1, x: 120, y: 350, name: "Size", initial: 0.5, bipolar: true,
    does: "Grain size, 10 ms to 4 s. 200 ms at the centre. At most half the recording.",
    value: (p) => ms(p[PARAM.grainMs]),
  },
  {
    id: "S32", type: "knob", index: 2, x: 240, y: 350, name: "Shape", initial: 0.5, bipolar: true,
    does: "Grain window: reverse percussive, Hann at the centre, percussive.",
    value: (p) => (Math.abs(p[PARAM.shape] - 0.5) < 0.01 ? "Hann" : `${p[PARAM.shape] > 0.5 ? "Perc" : "Rev"} ${percent(Math.abs(p[PARAM.shape] - 0.5) * 2)}`),
  },
  {
    id: "S33", type: "knob", index: 3, x: 358, y: 350, name: "Pitch", initial: 0.5, bipolar: true,
    does: "±24 semitones. Stops at 0, 0.1, 5, 7, 12, 17, 19, 24.",
    value: (p) => `${signed(p[PARAM.pitch], Math.abs(p[PARAM.pitch]) < 1 && p[PARAM.pitch] !== 0 ? 1 : 0)} st`,
  },
  {
    id: "S34", type: "knob", index: 4, x: 476, y: 350, name: "Density", initial: 0.7,
    does: "Grains per second. None at the bottom, then 0.3 to 131, exponential.",
    value: (p) => {
      const d = p[PARAM.density];
      return d === 0 ? "Off" : `${d.toFixed(d < 10 ? 1 : 0)}/s`;
    },
  },
  {
    id: "P10", type: "pad", index: RECORD_PAD, ...padAt(RECORD_PAD), name: "Record",
    does: "Tap: the last seconds, as set. Hold: up to 30 s. With Shift: listen, firmer for more.",
    value: (p, v) => (v.recording ? `● ${v.recording.toFixed(1)} s` : p[PARAM.mix] < 1 ? `Listen ${percent(1 - p[PARAM.mix])}` : v.sound ? `${seconds(v.sound)} held` : "Empty"),
  },
  {
    id: "S09", type: "switch", index: 0, x: 222, y: 638, name: "Direction", initial: POSITION.up,
    does: "Up forward, centre freeze, down reverse. Playhead and slices.",
    value: (p) => (p[PARAM.freeze] ? "Freeze" : p[PARAM.speed] < 0 ? "Reverse" : "Forward"),
  },
  ...SLICE_PADS.map((pad, i): Control => ({
    id: `P${String(pad).padStart(2, "0")}`, type: "pad", index: pad, ...padAt(pad), name: `Slice ${i + 1}`, slice: i,
    does: "Plays its slice with its own grains, as Trigger sets. Pressure sets density.",
    value: (_, v) => (v.slices[i] ? seconds(v.slices[i]) : ""),
  })),
  // P11 held, shift: each knob's randomness
  ...([
    ["S30", "Position", PARAM.randPos, "per grain"],
    ["S31", "Size", PARAM.randSize, "per grain"],
    ["S32", "Shape", PARAM.randShape, "per grain"],
    ["S33", "Pitch", PARAM.randPitch, "per grain"],
    ["S34", "Density", PARAM.randDensity, "of the starts"],
    ["S35", "Length", PARAM.randLength, "per pass"],
  ] as const).map(([knob, of, param, per]): Control => ({
    id: `P11+${knob}`, type: "knob", index: Number(knob.slice(1)) - 30, x: 0, y: 0, name: `${of} Random`, short: "Random", initial: 0.5, bipolar: true, layer: LAYER.shift, board: `P11 + ${knob}`,
    does: `${of} randomness ${per}. Off at the centre, up even, down a long tail.`,
    value: (p) => random(p[param]),
  })),
  {
    id: "P11+S37", type: "fader", index: 7, x: 0, y: 0, name: "Reverb", initial: 0.25, layer: LAYER.shift, board: "P11 + S37",
    does: "Reverb on the grains. Amount and decay, 0.5 to 16 s.",
    value: (p) => percent(p[PARAM.reverb]),
  },
  { id: "P11", type: "pad", index: SHIFT_PAD, ...padAt(SHIFT_PAD), name: "Shift", does: "Held: the knobs set their randomness, S37 the reverb. With Rec: listen." },
  {
    id: "S07", type: "switch", index: 1, x: 330, y: 690, name: "Trigger", initial: POSITION.up,
    does: "Slice pads: up once to the end, centre while held, down looped while held.",
    value: () => "",
  },
  {
    id: "P02", type: "pad", index: RESTART_PAD, ...padAt(RESTART_PAD), name: "Retrig",
    does: "Playhead to the selection start.",
  },
  {
    id: "P00", type: "pad", index: MUTE_PAD, ...padAt(MUTE_PAD), name: "Mute",
    does: "Held: the grains silent, the reverb rings on.",
  },
  { id: "P01", type: "pad", index: 1, ...padAt(1), name: "Free", does: "No function yet.", free: true },
];

export const byId = (id: string) => CONTROLS.find((c) => c.id === id)!;
export const padOf = (index: number) => CONTROLS.find((c) => c.type === "pad" && c.index === index)!;

export const LED: [string, string][] = [
  ["Glow", "Grains playing"],
  ["Flashing", "Recording"],
  ["Pulse", "At a centre: pitch unison, speed 1×, size 200 ms"],
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
    does: "Input level, ±12 dB. Browser only.",
    gain: (x) => 10 ** (((x - 0.5) * 24) / 20),
  },
  {
    id: "OUT", type: "knob", index: -1, x: 0, y: 0, name: "Out", initial: 1,
    does: "Output level, −60 to 0 dB, off at the bottom. Browser only.",
    gain: (x) => (x <= 0 ? 0 : 10 ** ((60 * (x - 1)) / 20)),
  },
];

// The levels' knob positions, as the page last set them
export const LEVELS_VALUE = new Map<string, number>();
for (const l of LEVELS) l.value = () => db(l.gain(LEVELS_VALUE.get(l.id) ?? l.initial ?? 0));
