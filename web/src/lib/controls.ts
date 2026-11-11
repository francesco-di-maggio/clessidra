// The Simple Touch's controls in this firmware: where each sits on the board,
// what it does, and how its value reads. Indices follow Controls
// (src/hw/controls.h). Positions are the board's, in the 600 × 973 photo's
// pixels (touch.jpeg). Play, Controls and the panel drawing read them.

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
const END = ["Once", "Turn", "Loop"]; // Playhead's End
// Shape's four, evenly along the knob, and their short names
const SHAPES = [["Square", "Sq"], ["Decay", "Dec"], ["Bell", "Bell"], ["Reverse", "Rev"]];

// Speed's stops from the bottom, as CoreUI's Stops(): each flat over the first
// third of its segment, marked at the middle of it
const SPEED_STOPS = ["0", "¼", "½", "1×", "2×", "4×"];
const SEGMENTS = SPEED_STOPS.length - 1;
const speedAt = (stop: number) => (stop === SEGMENTS ? SEGMENTS : stop === 0 ? 0 : stop + 1 / 6) / SEGMENTS;
const SPEED_SCALE = SPEED_STOPS.map((label, i) => ({ at: speedAt(i), label })).reverse();
// Density's grains per second where the fader puts them (CoreUI:
// kDensityNone, kMinDensity, kMaxDensity)
const NONE = 0.04;
const MIN_DENSITY = 0.25;
const MAX_DENSITY = 320;
const densityAt = (rate: number) => NONE + (1 - NONE) * (Math.log(rate / MIN_DENSITY) / Math.log(MAX_DENSITY / MIN_DENSITY));
const DENSITY_SCALE = [MAX_DENSITY, 100, 10, 1].map((r) => ({ at: densityAt(r), label: `${r}` })).concat([{ at: 0, label: "0" }]);
const db = (gain: number) => (gain < 0.001 ? "−∞ dB" : `${signed(20 * Math.log10(gain), 0)} dB`);

const PAD_AT: Record<number, [number, number]> = {
  0: [150, 620], 1: [310, 610], 2: [412, 625], 3: [48, 800], 4: [170, 765], 5: [285, 795],
  6: [420, 788], 7: [555, 835], 8: [190, 915], 9: [405, 920], 10: [240, 50], 11: [370, 50],
};
const padAt = (index: number) => ({ x: PAD_AT[index][0], y: PAD_AT[index][1] });

export const CONTROLS: Control[] = [
  {
    id: "S36", type: "fader", index: 6, x: 36, y: 0, name: "Speed", initial: speedAt(3), scale: SPEED_SCALE,
    does: "0 to 4×, with stops at ¼, ½, 1, 2 and 4×. The pitch stays. Frozen: scrubs the loop.",
    value: (p) => (p[PARAM.freeze] ? "Frozen" : `${Math.abs(p[PARAM.speed]).toFixed(2)}×`),
    twin: { name: "Volume", does: "Output level, off to 0 dB.", value: (p) => db(p[PARAM.volume]) },
  },
  {
    id: "S37", type: "fader", index: 7, x: 564, y: 0, name: "Density", initial: densityAt(20), scale: DENSITY_SCALE,
    does: "Grains per second, 0 to 320. At the top with Size at its centre, all 32 voices sound.",
    value: (p) => {
      const d = p[PARAM.density];
      return d === 0 ? "None" : `${d.toFixed(d < 10 ? 1 : 0)}/s`;
    },
    twin: { name: "Random timing", short: "Rnd time", does: "When grains start, from a regular pulse to random.", value: (p) => (p[PARAM.randTime] === 0 ? "Regular" : percent(p[PARAM.randTime])) },
  },
  {
    id: "S30", type: "knob", index: 0, x: 120, y: 478, name: "Position", initial: 0,
    does: "Where the loop starts. The playhead moves with it.",
    value: (p, v) => seconds(p[PARAM.start] * v.sound),
    twin: {
      name: "Filter", bipolar: true, does: "Low-pass below the centre, off at the centre, high-pass above.",
      value: (p) => {
        const f = p[PARAM.filter];
        return f === 0 ? "Off" : f < 0 ? `LP ${percent(-f)}` : `HP ${percent(f)}`;
      },
    },
  },
  {
    id: "S35", type: "knob", index: 5, x: 476, y: 478, name: "Length", initial: 1,
    does: "How much of the recording after Position the loop takes.",
    value: (p, v) => seconds(p[PARAM.length] * v.sound),
    twin: { name: "Reverb", does: "Reverb on the grains, from none to long tails.", value: (p) => percent(p[PARAM.reverb]) },
  },
  {
    id: "S31", type: "knob", index: 1, x: 120, y: 350, name: "Size", initial: 0.5, bipolar: true, detent: true,
    does: "Grain length, 5 ms to 2 s, 100 ms at the centre.",
    value: (p) => ms(p[PARAM.grainMs]),
    twin: { name: "Random size", short: "Rnd size", does: "Each grain a different length, up to about an octave either way.", value: (p) => (p[PARAM.randSize] === 0 ? "Off" : percent(Math.sqrt(4 * p[PARAM.randSize]))) },
  },
  {
    id: "S32", type: "knob", index: 2, x: 240, y: 350, name: "Spray", initial: 0,
    does: "How far grains stray from the playhead, up to the whole loop, and across the stereo field.",
    value: (p) => (p[PARAM.spread] === 0 ? "Off" : percent(p[PARAM.spread])),
    twin: { name: "Reverse", does: "How many grains play backwards: none, some, all.", value: (p) => (p[PARAM.reverse] === 0 ? "Off" : percent(p[PARAM.reverse])) },
  },
  {
    id: "S33", type: "knob", index: 3, x: 358, y: 350, name: "Pitch", initial: 0.5, bipolar: true, detent: true,
    does: "Transpose up to two octaves either way, with stops at musical intervals.",
    value: (p) => `${signed(p[PARAM.pitch], Math.abs(p[PARAM.pitch]) < 1 && p[PARAM.pitch] !== 0 ? 1 : 0)} st`,
    twin: {
      name: "Random pitch", short: "Rnd pitch", bipolar: true, does: "Off at the centre. Above: any pitch up to an octave either way. Below: octaves only.",
      value: (p) => {
        const r = p[PARAM.randPitch];
        return r === 0 ? "Off" : r < 0 ? `Oct ${percent(-r)}` : `±${(r * RAND_PITCH).toFixed(1)} st`;
      },
    },
  },
  {
    id: "S34", type: "knob", index: 4, x: 476, y: 350, name: "Shape", initial: 2 / 3,
    does: "Grain envelope: square, decay, bell, reverse, and everything between.",
    value: (p) => {
      // At a shape, its name. Between two, both.
      const at = p[PARAM.shape] * (SHAPES.length - 1);
      const i = Math.min(Math.floor(at + 0.04), SHAPES.length - 1);
      return Math.abs(at - i) < 0.04 ? SHAPES[i][0] : `${SHAPES[i][1]} › ${SHAPES[i + 1][1]}`;
    },
    twin: { name: "Random shape", short: "Rnd shape", does: "Each grain a different envelope.", value: (p) => (p[PARAM.randShape] === 0 ? "Off" : percent(p[PARAM.randShape])) },
  },
  {
    id: "S09", type: "switch", index: 0, x: 222, y: 638, name: "Direction", initial: POSITION.up,
    does: "Forward, freeze or reverse. Frozen, Speed scrubs through the loop.",
    value: (p) => (p[PARAM.freeze] ? "Frozen" : p[PARAM.speed] < 0 ? "Reverse" : "Forward"),
  },
  {
    id: "S07", type: "switch", index: 1, x: 330, y: 690, name: "End", initial: POSITION.up,
    does: "At the loop's end: play once, turn over and run back, or loop. Slices follow it.",
    value: (p) => END[p[PARAM.end]] ?? "",
  },
  {
    id: "P10", type: "pad", index: RECORD_PAD, ...padAt(RECORD_PAD), name: "Record",
    does: "Tap: keeps the last 8 s. Hold: records up to 30 s. With Shift: hear the input.",
    value: (_, v) => (v.recording ? `● ${v.recording.toFixed(1)} s` : v.sound ? `${seconds(v.sound)} held` : "Empty"),
  },
  {
    id: "P11", type: "pad", index: SHIFT_PAD, ...padAt(SHIFT_PAD), name: "Shift",
    does: "Hold for each knob's second function.",
  },
  {
    id: "P01", type: "pad", index: RESET_PAD, ...padAt(RESET_PAD), name: "Reset",
    does: "Second functions back to their defaults.",
  },
  {
    id: "P00", type: "pad", index: STOP_PAD, ...padAt(STOP_PAD), name: "Stop",
    does: "Held: stops as a tape machine, slowing to a halt. Released: back to speed.",
    value: (p) => (p[PARAM.stop] ? "Stopped" : ""),
  },
  {
    id: "P02", type: "pad", index: TRIG_PAD, ...padAt(TRIG_PAD), name: "Trig",
    does: "Plays the loop from its start, at once.",
  },
  ...SLICE_PADS.map((index, i): Control => ({
    id: `P${String(index).padStart(2, "0")}`, type: "pad", index, ...padAt(index), name: `Slice ${i + 1}`,
    does: "Hold to play a slice, cut at the recording's attacks. Several at once.",
  })),
];

export const byId = (id: string) => CONTROLS.find((c) => c.id === id)!;
export const padOf = (index: number) => CONTROLS.find((c) => c.type === "pad" && c.index === index)!;

// Starting points: a sound and how to set the panel for it
export const PLAY: [string, string][] = [
  ["Rhythm", "Density low, Random timing (P11 + S37) down, Shape on decay, Size short. Size sets how long each hit lasts"],
  ["Drops", "Density near the bottom, Size short: single grains, now and then"],
  ["Drums by hand", "Density at the bottom, Shape on square: tap the slices, several at once"],
  ["Cloud", "Density at the top, Size 150 to 300 ms, Shape on bell, Spray up, Random timing (P11 + S37) full, Reverb (P11 + S35) up"],
  ["Stretch", "Speed at ¼ or ½, Density up. S09 down to hear it backwards"],
  ["Drone", "S09 at the centre, Density at the top, Size long, a little Spray, Reverb up. Speed scrubs the point"],
  ["One pour", "S07 down: the loop plays once and stops. Trig pours it again"],
  ["Shimmer", "Random pitch (P11 + S33) below the centre: octaves"],
  ["Backwards syllables", "Reverse (P11 + S32) at the top: the phrase in order, each grain backwards"],
];

export const LED: [string, string][] = [
  ["Glow", "Grains falling"],
  ["Flashing", "Recording"],
  ["Pulse", "At a stop: unison, speed 1×, 100 ms, filter off, random pitch off"],
  ["Steady", "P11 held: the twin last turned"],
];

// Browser only: the level into the firmware and out of it. The board has its
// own, on the hardware.
export interface Level extends Control {
  gain: (x: number) => number; // linear
}

export const LEVELS: Level[] = [
  {
    id: "OUT", type: "knob", index: -1, x: 0, y: 0, name: "Out", initial: 1,
    does: "Output level, off to 0 dB. In the browser only.",
    gain: (x) => (x <= 0 ? 0 : 10 ** ((60 * (x - 1)) / 20)),
  },
];

// The levels' knob positions, as the page last set them
export const LEVELS_VALUE = new Map<string, number>();
for (const l of LEVELS) l.value = () => db(l.gain(LEVELS_VALUE.get(l.id) ?? l.initial ?? 0));
