// The Simple Touch's controls in this firmware: where each sits on the board,
// what it does, and how its value reads. Indices follow Controls
// (src/hw/controls.h). Positions are the board's, in the 600 × 973 photo's
// pixels (touch.jpeg). Play, Mapping and the panel drawing read them.

import { TRIG_PAD, PARAM, RATE, RECORD_PAD, RESET_PAD, SHIFT_PAD, SLICE_PADS, STOP_PAD } from "./engine";

export const POSITION = { centre: 0, up: 1, down: 2 } as const; // libDaisy's Switch3

// What a readout knows besides the parameters
export interface State {
  sound: number; // samples recorded
  recording: number; // seconds P10 has been held, 0 when not
}

// A knob's function with P11 held
export interface Twin {
  name: string;
  short?: string; // where room is short
  bipolar?: boolean; // drawn from its centre
  does: string;
  value: (p: Float32Array) => string;
}

export interface Control {
  id: string;
  type: "knob" | "fader" | "switch" | "pad";
  index: number; // Controls' knob, switch or pad
  name: string;
  short?: string; // where room is short
  x: number;
  y: number;
  initial?: number;
  bipolar?: boolean; // drawn from its centre
  detent?: boolean; // stops at its centre
  scale?: { at: number; label: string }[]; // a fader's marks, 0..1 of its travel
  free?: boolean; // not assigned yet
  does?: string;
  value?: (p: Float32Array, v: State) => string;
  twin?: Twin;
}

const seconds = (samples: number) => `${(samples / RATE).toFixed(samples < RATE ? 2 : 1)} s`;
const percent = (x: number) => `${Math.round(x * 100)} %`;
const ms = (x: number) => (x < 1000 ? `${Math.round(x)} ms` : `${(x / 1000).toFixed(2)} s`);
const signed = (x: number, digits: number) => `${x > 0 ? "+" : x < 0 ? "−" : ""}${Math.abs(x).toFixed(digits)}`;
const RAND_PITCH = 12; // Core::kRandPitchSemitones
const QUANTIZE = ["Off", "Semitones", "Octaves"]; // Core's Quantize

// Speed's stops from the bottom, as CoreUI's Stops(): each flat over the first
// third of its segment, marked at the middle of it
const SPEED_STOPS = ["0", "¼", "½", "1×", "2×", "4×"];
const SEGMENTS = SPEED_STOPS.length - 1;
const speedAt = (stop: number) => (stop === SEGMENTS ? SEGMENTS : stop === 0 ? 0 : stop + 1 / 6) / SEGMENTS;
const SPEED_SCALE = SPEED_STOPS.map((label, i) => ({ at: speedAt(i), label })).reverse();
// Density's grains at once where the fader puts them, either side of the
// centre (CoreUI: kDensityDetent, kMinOverlap, kMaxOverlap)
const DETENT = 0.04;
const overlapAt = (overlap: number, sign: number) => 0.5 + sign * (DETENT + (0.5 - DETENT) * (Math.log(overlap * 16) / Math.log(256)));
const DENSITY_SCALE = [16, 4, 1].map((o) => ({ at: overlapAt(o, 1), label: `${o}` }))
  .concat([{ at: 0.5, label: "0" }])
  .concat([1, 4, 16].map((o) => ({ at: overlapAt(o, -1), label: `${o}` })));
const fraction = (x: number) => (x >= 1 ? x.toFixed(1) : `1/${Math.round(1 / x)}`);

const PAD_AT: Record<number, [number, number]> = {
  0: [150, 620], 1: [310, 610], 2: [412, 625], 3: [48, 800], 4: [170, 765], 5: [285, 795],
  6: [420, 788], 7: [555, 835], 8: [190, 915], 9: [405, 920], 10: [240, 50], 11: [370, 50],
};
const padAt = (index: number) => ({ x: PAD_AT[index][0], y: PAD_AT[index][1] });

export const CONTROLS: Control[] = [
  {
    id: "S36", type: "fader", index: 6, x: 36, y: 0, name: "Speed", initial: speedAt(3), scale: SPEED_SCALE,
    does: "Playback speed, 0 to 4×, the pitch kept. Frozen, it scrubs through the loop.",
    value: (p) => (p[PARAM.freeze] ? "Frozen" : `${signed(p[PARAM.speed], 2)}×`),
    twin: { name: "Dry/wet", short: "Mix", does: "The input against the grains, dry to wet.", value: (p) => percent(p[PARAM.mix]) },
  },
  {
    id: "S37", type: "fader", index: 7, x: 564, y: 0, name: "Density", initial: 0.8, bipolar: true, detent: true, scale: DENSITY_SCALE,
    does: "Grains at once, up to 16. None at the centre, regular below, random above.",
    value: (p) => {
      const d = p[PARAM.density];
      if (d === 0) return "None";
      return `${p[PARAM.randomStarts] ? "Random" : "Regular"} ${fraction((d * p[PARAM.grainMsSet]) / 1000)}`;
    },
    twin: { name: "Reverb", does: "Reverb on the grains, from none to long tails.", value: (p) => percent(p[PARAM.reverb]) },
  },
  {
    id: "S30", type: "knob", index: 0, x: 120, y: 478, name: "Position", initial: 0,
    does: "Where the loop starts. The playhead moves with it.",
    value: (p, v) => seconds(p[PARAM.start] * v.sound),
  },
  {
    id: "S35", type: "knob", index: 5, x: 476, y: 478, name: "Length", initial: 1,
    does: "How much of the rest the loop takes: at the top, to the end.",
    value: (p, v) => seconds(p[PARAM.length] * v.sound),
  },
  {
    id: "S31", type: "knob", index: 1, x: 120, y: 350, name: "Size", initial: 0.5, bipolar: true, detent: true,
    does: "Grain length, 5 ms to 2 s, 100 ms at the centre.",
    value: (p) => ms(p[PARAM.grainMs]),
    twin: { name: "Random size", short: "Rnd size", does: "Each grain a different length, up to two octaves either way.", value: (p) => (p[PARAM.randSize] === 0 ? "Off" : percent(Math.sqrt(4 * p[PARAM.randSize]))) },
  },
  {
    id: "S32", type: "knob", index: 2, x: 240, y: 350, name: "Random position", short: "Rnd pos", initial: 0,
    does: "How far grains stray from the playhead, up to the whole loop, and how wide across the stereo field.",
    value: (p) => (p[PARAM.spread] === 0 ? "Off" : percent(p[PARAM.spread])),
  },
  {
    id: "S33", type: "knob", index: 3, x: 358, y: 350, name: "Pitch", initial: 0.5, bipolar: true, detent: true,
    does: "Transpose up to two octaves either way, with stops at musical intervals.",
    value: (p) => `${signed(p[PARAM.pitch], Math.abs(p[PARAM.pitch]) < 1 && p[PARAM.pitch] !== 0 ? 1 : 0)} st`,
    twin: { name: "Random pitch", short: "Rnd pitch", does: "Each grain a different pitch, up to an octave, snapped by Quantize.", value: (p) => (p[PARAM.randPitch] === 0 ? "Off" : `±${(p[PARAM.randPitch] * RAND_PITCH).toFixed(1)} st`) },
  },
  {
    id: "S34", type: "knob", index: 4, x: 476, y: 350, name: "Shape", initial: 0.5, bipolar: true, detent: true,
    does: "Grain envelope: reversed attack, smooth at the centre, sharp attack.",
    value: (p) => {
      const s = p[PARAM.shape];
      return s === 0.5 ? "Hann" : s < 0.5 ? `Rev ${percent(1 - 2 * s)}` : `Perc ${percent(2 * s - 1)}`;
    },
    twin: {
      name: "Filter", bipolar: true, does: "Low-pass below the centre, off at the centre, high-pass above.",
      value: (p) => {
        const f = p[PARAM.filter];
        return f === 0 ? "Off" : f < 0 ? `LP ${percent(-f)}` : `HP ${percent(f)}`;
      },
    },
  },
  {
    id: "S09", type: "switch", index: 0, x: 222, y: 638, name: "Mode", initial: POSITION.up,
    does: "Forward, freeze or reverse. Frozen, Speed scrubs through the loop.",
    value: (p) => (p[PARAM.freeze] ? "Frozen" : p[PARAM.speed] < 0 ? "Reverse" : "Forward"),
  },
  {
    id: "S07", type: "switch", index: 1, x: 330, y: 690, name: "Quantize", initial: POSITION.down,
    does: "Snaps random pitch to octaves or semitones, or leaves it free.",
    value: (p) => QUANTIZE[p[PARAM.quantize]] ?? "",
  },
  {
    id: "P10", type: "pad", index: RECORD_PAD, ...padAt(RECORD_PAD), name: "Record",
    does: "Tap to keep the last 8 s, hold to record up to 30 s. With Shift: listen.",
    value: (_, v) => (v.recording ? `● ${v.recording.toFixed(1)} s` : v.sound ? `${seconds(v.sound)} held` : "Empty"),
  },
  {
    id: "P11", type: "pad", index: SHIFT_PAD, ...padAt(SHIFT_PAD), name: "Shift",
    does: "Hold for each knob’s second function, shown in orange.",
  },
  {
    id: "P01", type: "pad", index: RESET_PAD, ...padAt(RESET_PAD), name: "Reset",
    does: "Second functions back to their defaults.",
  },
  {
    id: "P00", type: "pad", index: STOP_PAD, ...padAt(STOP_PAD), name: "Stop",
    does: "Stops as a tape machine, slowing to a halt. Touch again to start.",
    value: (p) => (p[PARAM.stop] ? "Stopped" : ""),
  },
  {
    id: "P02", type: "pad", index: TRIG_PAD, ...padAt(TRIG_PAD), name: "Trig",
    does: "Playback again from the loop's start, with a grain at once: back on the beat.",
  },
  ...SLICE_PADS.map((index, i): Control => ({
    id: `P${String(index).padStart(2, "0")}`, type: "pad", index, ...padAt(index), name: `Slice ${i + 1}`,
    does: "Held: plays its slice, round it, in the direction set. Several at once. The loop waits.",
  })),
];

export const byId = (id: string) => CONTROLS.find((c) => c.id === id)!;
export const padOf = (index: number) => CONTROLS.find((c) => c.type === "pad" && c.index === index)!;

// Starting points: a sound and how to set the panel for it
export const PLAY: [string, string][] = [
  ["Rhythm", "Density below the centre, Shape up, Size short. Tap the slices: each restarts the beat"],
  ["Drums by hand", "Density at the centre: each slice plays one grain"],
  ["Cloud", "Density at the top, Random position up, Size long, Reverb (P11 + S37) up"],
  ["Stretch", "Speed at ¼ or ½, Density up. S09 down to hear it backwards"],
  ["A moment held", "S09 at the centre, Density up, a little Random position. Speed scrubs the point"],
  ["Shimmer", "Random pitch (P11 + S33) up, S07 up: octaves"],
];

export const LED: [string, string][] = [
  ["Glow", "Grains falling, dimmer as the sand runs out"],
  ["Flashing", "Recording"],
  ["Pulse", "At a stop: unison, speed 1×, 100 ms, Hann, filter off"],
  ["Steady", "P11 held: the twin last turned"],
];

// Browser only: the level into the firmware and out of it. The board has its
// own, on the hardware.
export interface Level extends Control {
  gain: (x: number) => number; // linear
}

const db = (gain: number) => (gain < 0.001 ? "−∞ dB" : `${signed(20 * Math.log10(gain), 0)} dB`);

export const LEVELS: Level[] = [
  {
    id: "IN", type: "knob", index: -1, x: 0, y: 0, name: "In", initial: 0.5, bipolar: true,
    does: "Input level, ±12 dB. In the browser only.",
    gain: (x) => 10 ** (((x - 0.5) * 24) / 20),
  },
  {
    id: "OUT", type: "knob", index: -1, x: 0, y: 0, name: "Out", initial: 1,
    does: "Output level, off to 0 dB. In the browser only.",
    gain: (x) => (x <= 0 ? 0 : 10 ** ((60 * (x - 1)) / 20)),
  },
];

// The levels' knob positions, as the page last set them
export const LEVELS_VALUE = new Map<string, number>();
for (const l of LEVELS) l.value = () => db(l.gain(LEVELS_VALUE.get(l.id) ?? l.initial ?? 0));
