// The Simple Touch's controls in this firmware: where each sits on the board,
// what it does, and how its value reads. Indices follow Controls
// (src/hw/controls.h). Positions are the board's, in the 600 × 973 photo's
// pixels (touch.jpeg). Play, Mapping and the panel drawing read them.

import { HOLD_PAD, MUTE_PAD, PARAM, PLACE_PADS, RATE, RECORD_PAD, SHAKE_PAD, TURN_PAD } from "./engine";

export const POSITION = { centre: 0, up: 1, down: 2 } as const; // libDaisy's Switch3

// What a readout knows besides the parameters
export interface Live {
  sound: number; // samples recorded
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
  scale?: { at: number; label: string }[]; // a fader's marks, 0..1 of its travel
  does?: string;
  value?: (p: Float32Array, v: Live) => string;
}

const seconds = (samples: number) => `${(samples / RATE).toFixed(samples < RATE ? 2 : 1)} s`;
const percent = (x: number) => `${Math.round(x * 100)} %`;
const ms = (x: number) => (x < 1000 ? `${Math.round(x)} ms` : `${(x / 1000).toFixed(2)} s`);
const signed = (x: number, digits: number) => `${x > 0 ? "+" : x < 0 ? "−" : ""}${Math.abs(x).toFixed(digits)}`;
const END = ["Once", "Turn", "Loop"]; // Core's End
const REVERB = { short: 0.35, long: 0.8 }; // CoreUI::kReverbShort, kReverbLong

// Speed's stops, as CoreUI's Stops(): each flat over the first third of its
// segment, marked at the middle of it
const SPEED_STOPS = ["¼", "½", "1×", "2×", "4×"];
const SEGMENTS = SPEED_STOPS.length;
const speedAt = (stop: number, sign: number) => 0.5 + (sign * (stop === SEGMENTS ? SEGMENTS : stop + 1 / 6)) / (2 * SEGMENTS);
const SPEED_SCALE = SPEED_STOPS.map((label, i) => ({ at: speedAt(i + 1, 1), label }))
  .reverse()
  .concat([{ at: 0.5, label: "0" }])
  .concat(SPEED_STOPS.map((label, i) => ({ at: speedAt(i + 1, -1), label })));
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
    id: "S36", type: "fader", index: 6, x: 36, y: 0, name: "Speed", initial: speedAt(3, 1), bipolar: true, detent: true, scale: SPEED_SCALE,
    does: "How fast the sand runs: the time stretched, the pitch kept. Held at the centre. Up forward, down reverse. Stops at ¼, ½, 1, 2 and 4×.",
    value: (p) => (p[PARAM.freeze] ? "Held" : `${signed(p[PARAM.speed], 2)}×`),
  },
  {
    id: "S37", type: "fader", index: 7, x: 564, y: 0, name: "Density", initial: 0.8, bipolar: true, detent: true, scale: DENSITY_SCALE,
    does: "Grains at once, 1/16 to 16. None at the centre. Down: regular, drops to a stream. Up: random, scattered to a cloud.",
    value: (p) => {
      const d = p[PARAM.density];
      if (d === 0) return "None";
      return `${p[PARAM.randomStarts] ? "Random" : "Regular"} ${fraction((d * p[PARAM.grainMsSet]) / 1000)}`;
    },
  },
  {
    id: "S30", type: "knob", index: 0, x: 120, y: 478, name: "Position", initial: 0,
    does: "Where the sand starts, anywhere in the recording. Held: moves the point.",
    value: (p, v) => seconds(p[PARAM.start] * v.sound),
  },
  {
    id: "S35", type: "knob", index: 5, x: 476, y: 478, name: "Length", initial: 1,
    does: "How much sand: the selection, past the end round to the start.",
    value: (p, v) => seconds(p[PARAM.length] * v.sound),
  },
  {
    id: "S31", type: "knob", index: 1, x: 120, y: 350, name: "Size", initial: 0.5, bipolar: true, detent: true,
    does: "Grain length, 5 ms to 2 s, 100 ms at the centre, at most half the recording.",
    value: (p) => ms(p[PARAM.grainMs]),
  },
  {
    id: "S32", type: "knob", index: 2, x: 240, y: 350, name: "Spray", initial: 0,
    does: "How far grains stray from the playhead, evenly: position, then size, pitch, pan, level, reverb send.",
    value: (p) => (p[PARAM.spread] === 0 ? "Plain" : percent(p[PARAM.spread])),
  },
  {
    id: "S33", type: "knob", index: 3, x: 358, y: 350, name: "Pitch", initial: 0.5, bipolar: true, detent: true,
    does: "±24 semitones. Stops at 0, 1, 3, 4, 7, 12, 24.",
    value: (p) => `${signed(p[PARAM.pitch], Math.abs(p[PARAM.pitch]) < 1 && p[PARAM.pitch] !== 0 ? 1 : 0)} st`,
  },
  {
    id: "S34", type: "knob", index: 4, x: 476, y: 350, name: "Shape", initial: 0.5, bipolar: true, detent: true,
    does: "The grain's window: reverse percussive down, Hann at the centre, percussive up.",
    value: (p) => {
      const s = p[PARAM.shape];
      return s === 0.5 ? "Hann" : s < 0.5 ? `Rev ${percent(1 - 2 * s)}` : `Perc ${percent(2 * s - 1)}`;
    },
  },
  {
    id: "S09", type: "switch", index: 0, x: 222, y: 638, name: "Reverb", initial: POSITION.centre,
    does: "On the grains: down off, centre short (1.7 s), up long (8 s).",
    value: (p) => (p[PARAM.reverb] === 0 ? "Off" : p[PARAM.reverb] < REVERB.long ? "Short" : "Long"),
  },
  {
    id: "S07", type: "switch", index: 1, x: 330, y: 690, name: "End", initial: POSITION.down,
    does: "When the sand runs out: up stops, centre turns the glass over, down starts again.",
    value: (p) => END[p[PARAM.end]] ?? "",
  },
  {
    id: "P10", type: "pad", index: RECORD_PAD, ...padAt(RECORD_PAD), name: "Record",
    does: "Tap: the last 8 s. Hold: up to 30 s, the input heard. The new sand pours in over 2 s.",
    value: (_, v) => (v.recording ? `● ${v.recording.toFixed(1)} s` : v.sound ? `${seconds(v.sound)} held` : "Empty"),
  },
  {
    id: "P11", type: "pad", index: TURN_PAD, ...padAt(TURN_PAD), name: "Turn",
    does: "A touch turns the glass over: from where it is, the sand runs the other way, the grains backwards.",
    value: (p) => (p[PARAM.heading] < 0 ? "Turned" : "Upright"),
  },
  {
    id: "P00", type: "pad", index: SHAKE_PAD, ...padAt(SHAKE_PAD), name: "Shake",
    does: "Held: the sand shaken, Spray at its top.",
  },
  {
    id: "P01", type: "pad", index: MUTE_PAD, ...padAt(MUTE_PAD), name: "Mute",
    does: "Held: the grains silent, the reverb rings on.",
  },
  {
    id: "P02", type: "pad", index: HOLD_PAD, ...padAt(HOLD_PAD), name: "Hold",
    does: "Held: the sand stops, the grains stay on that moment. Released: it runs on.",
  },
  ...PLACE_PADS.map((index, i): Control => ({
    id: `P${String(index).padStart(2, "0")}`, type: "pad", index, ...padAt(index), name: `Place ${i + 1}`,
    does: `The selection in sevenths, left to right, this one at ${i}/7. A touch: the playhead there, a grain at once, the beat from it. Held: it stays there.`,
  })),
];

export const byId = (id: string) => CONTROLS.find((c) => c.id === id)!;
export const padOf = (index: number) => CONTROLS.find((c) => c.type === "pad" && c.index === index)!;

// Starting points: a sound and how to set the panel for it
export const PLAY: [string, string][] = [
  ["Rhythm", "Density below the centre, Shape up, Size short. Tap the places: each restarts the beat"],
  ["Drums by hand", "Density at the centre: each place plays one grain"],
  ["Cloud", "Density at the top, Spray up, Size long, Reverb long"],
  ["Stretch", "Speed at ¼ or ½, Density up. Turn to hear it backwards"],
  ["A moment held", "Hold, or a place held, Density up, a little Spray, Shake for a burst"],
];

export const LED: [string, string][] = [
  ["Glow", "Grains falling, dimmer as the sand runs out"],
  ["Flashing", "Recording"],
  ["Pulse", "At a stop: unison, speed 1× either way, 100 ms, Hann"],
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
