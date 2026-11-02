// The Simple Touch panel: where each control sits, what it does in this
// firmware, and the drawing that Play and Mapping share. Indices follow
// Controls (src/hw/controls.h). Positions are the board's, in the 600 × 973
// photo's pixels (touch.jpeg).

export const POSITION = { centre: 0, up: 1, down: 2 }; // as libDaisy's Switch3
export const END = ["One-shot", "Turn", "Loop"]; // Playhead::End
// Slices first to last in the buffer (CoreUI::kSlicePads)
export const SLICE_PADS = [3, 4, 8, 5, 6, 9, 7];

// cl_params fields
export const P = { speed: 0, freeze: 1, end: 2, density: 3, grainMs: 4, pitch: 5, fill: 6, reverb: 7, jitter: 8, randPitch: 9, shape: 10, start: 11, length: 12 };

const RATE = 48000;
const seconds = (samples) => `${(samples / RATE).toFixed(samples < RATE ? 2 : 1)} s`;
const percent = (x) => `${Math.round(x * 100)}\u00a0%`;
const signed = (x, digits) => `${x > 0 ? "+" : x < 0 ? "−" : ""}${Math.abs(x).toFixed(digits)}`;

// type: knob, fader, switch, pad. index: Controls' knob, switch or pad.
// value(p, v): the readout, from the parameters in use and the view
export const CONTROLS = [
  {
    id: "S36", type: "fader", index: 6, x: 36, name: "Tilt", initial: 0.75,
    does: "Centre stops, up forward, down backward, up to 4 times. Speed 1 half way.",
    value: (p) => (p[P.freeze] ? "Stopped" : `${p[P.speed] > 0 ? "Forward" : "Backward"} × ${Math.abs(p[P.speed]).toFixed(2)}`),
  },
  {
    id: "S37", type: "fader", index: 7, x: 564, name: "Pour", initial: 0.6,
    does: "Closed, single percussive drops in reverb, a stream, a wide cloud of long grains. Up to 130 grains per second.",
    value: (p) => (p[P.density] === 0 ? "Closed" : `${p[P.density].toFixed(p[P.density] < 10 ? 1 : 0)} grains/s`),
  },
  {
    id: "S30", type: "knob", index: 0, x: 120, y: 478, name: "Position", initial: 0,
    does: "How far back the selection lies from where the sound ends.",
    value: (p, v) => `${seconds(p[P.start] * v.sound)} back`,
  },
  {
    id: "S35", type: "knob", index: 5, x: 476, y: 478, name: "Length", initial: 1,
    does: "Length of the selection.",
    value: (p, v) => seconds(p[P.length] * v.sound),
  },
  {
    id: "S31", type: "knob", index: 1, x: 120, y: 350, name: "Size", initial: 0.5,
    does: "Grain size, 2 octaves either way from what Pour sets.",
    value: (p) => `${Math.round(p[P.grainMs])} ms`,
  },
  {
    id: "S32", type: "knob", index: 2, x: 240, y: 350, name: "Scatter", initial: 0,
    does: "From periodic and plain to random starts, positions and sizes. Above the centre, random pitch.",
    value: (p) => (p[P.randPitch] > 0 ? `${percent(p[P.jitter])} · pitch ${percent(p[P.randPitch])}` : percent(p[P.jitter])),
  },
  {
    id: "S33", type: "knob", index: 3, x: 358, y: 350, name: "Pitch", initial: 0.5,
    does: "±2 octaves, stops at 0, 0.1, 1, 3, 4, 7, 12 and 24 semitones.",
    value: (p) => `${signed(p[P.pitch], Math.abs(p[P.pitch]) < 1 && p[P.pitch] !== 0 ? 1 : 0)} st`,
  },
  {
    id: "S34", type: "knob", index: 4, x: 476, y: 350, name: "Fill", initial: 1,
    does: "How much of the input replaces the sound, and the odds that a grain follows the input. Down: sealed, the sound stays. Up: you hear the input through the grains, about 50 ms late.",
    value: (p) => (p[P.fill] === 0 ? "Sealed" : p[P.fill] === 1 ? "Live" : percent(p[P.fill])),
  },
  {
    id: "S07", type: "switch", index: 1, x: 330, y: 690, name: "End", initial: POSITION.centre,
    does: "What the playhead does at the end of the selection: one-shot (up), turn (centre), loop (down).",
    value: (p) => END[p[P.end]],
  },
  ...SLICE_PADS.map((pad, i) => ({
    id: `P0${pad}`, type: "pad", index: pad, name: `Slice ${i + 1}`, slice: i,
    does: "Slices at the sound's onsets. A touch plays one, a firm press opens Pour.",
    value: (p, v) => (v.slice === i ? "Playing" : ""),
  })),
  { id: "S09", type: "switch", index: 0, x: 222, y: 638, name: "Free", free: true, initial: POSITION.centre, label: "below" },
  { id: "P00", type: "pad", index: 0, name: "Free", free: true },
  { id: "P01", type: "pad", index: 1, name: "Free", free: true },
  { id: "P02", type: "pad", index: 2, name: "Free", free: true },
  { id: "P10", type: "pad", index: 10, name: "Free", free: true },
  { id: "P11", type: "pad", index: 11, name: "Free", free: true },
];

const PAD_AT = {
  0: [150, 620], 1: [310, 610], 2: [412, 625], 3: [48, 800], 4: [170, 765], 5: [285, 795],
  6: [420, 788], 7: [555, 835], 8: [190, 915], 9: [405, 920], 10: [240, 50], 11: [370, 50],
};
for (const c of CONTROLS) if (c.type === "pad") [c.x, c.y] = PAD_AT[c.index];

export const LED = [
  ["Glow", "Grains playing"],
  ["Pulse", "A control at or turning within its centre: pitch unison, Tilt stopped or at speed 1, size at its centre, Fill sealed"],
];

// Drawing
const SVG = "http://www.w3.org/2000/svg";
const KNOB_R = 40;
const SWEEP = 270; // degrees, from 7:30 to 4:30
const FADER = { top: 300, bottom: 545 };
const PAD_R = 40;

function el(name, attrs = {}, parent) {
  const e = document.createElementNS(SVG, name);
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, v);
  parent?.append(e);
  return e;
}

function text(parent, cls, x, y, content) {
  const t = el("text", { class: cls, x, y }, parent);
  t.textContent = content;
  return t;
}

function polar(cx, cy, r, deg) {
  const a = ((deg - 90) * Math.PI) / 180;
  return [cx + r * Math.cos(a), cy + r * Math.sin(a)];
}

function arc(cx, cy, r, from, to) {
  const [x0, y0] = polar(cx, cy, r, from);
  const [x1, y1] = polar(cx, cy, r, to);
  return `M${x0} ${y0}A${r} ${r} 0 ${to - from > 180 ? 1 : 0} 1 ${x1} ${y1}`;
}

// The panel as an SVG, with a draw(id, value) per control. Interactive: the
// controls take pointer and keys, and report through onInput(control, value)
// and onPad(control, touched).
export class Panel {
  constructor(svg, { interactive = false, onInput, onPad, onFocus } = {}) {
    this.svg = svg;
    this.parts = new Map();
    this.values = new Map();
    this.onInput = onInput;
    this.onPad = onPad;
    this.onFocus = onFocus;
    this.interactive = interactive;
    svg.setAttribute("viewBox", "0 0 600 973");
    el("rect", { class: "plate", x: 2, y: 2, width: 596, height: 969, rx: 18 }, svg);
    for (const [x, y] of [[50, 26], [550, 26], [50, 947], [550, 947]]) el("rect", { class: "hole", x: x - 18, y: y - 9, width: 36, height: 18, rx: 9 }, svg);
    for (const [y, label] of [[118, "IN"], [196, "OUT"]]) {
      el("circle", { class: "jack", cx: 34, cy: y, r: 13 }, svg);
      text(svg, "code", 34, y + 34, label);
    }
    for (const c of CONTROLS) this.add(c);
  }

  add(c) {
    const g = el("g", { class: `ctl ${c.type}${c.free ? " free" : ""}`, "data-id": c.id }, this.svg);
    const part = { c, g };
    if (c.type === "knob") {
      el("circle", { class: "hit", cx: c.x, cy: c.y, r: KNOB_R + 12 }, g);
      el("path", { class: "track", d: arc(c.x, c.y, KNOB_R, -SWEEP / 2, SWEEP / 2), style: "stroke-width:4" }, g);
      part.arc = el("path", { class: "arc" }, g);
      el("circle", { class: "ring", cx: c.x, cy: c.y, r: KNOB_R - 9 }, g);
      part.dot = el("circle", { class: "lever", r: 4 }, g);
      text(g, "code", c.x, c.y + 5, c.id);
      text(g, "name", c.x, c.y + KNOB_R + 24, c.name);
    } else if (c.type === "fader") {
      el("rect", { class: "hit", x: c.x - 28, y: FADER.top - 20, width: 56, height: FADER.bottom - FADER.top + 40 }, g);
      el("line", { class: "track", x1: c.x, x2: c.x, y1: FADER.top, y2: FADER.bottom }, g);
      el("line", { class: "tick", x1: c.x - 16, x2: c.x - 9, y1: (FADER.top + FADER.bottom) / 2, y2: (FADER.top + FADER.bottom) / 2 }, g);
      part.travel = el("line", { class: "travel", x1: c.x, x2: c.x, y2: FADER.bottom }, g);
      part.cap = el("rect", { class: "cap", x: c.x - 22, width: 44, height: 20, rx: 5 }, g);
      text(g, "code", c.x, FADER.top - 22, c.id);
      text(g, "name", c.x, FADER.bottom + 38, c.name);
    } else if (c.type === "switch") {
      el("rect", { class: "hit", x: c.x - 26, y: c.y - 40, width: 52, height: 80 }, g);
      el("rect", { class: "slot", x: c.x - 14, y: c.y - 30, width: 28, height: 60, rx: 8 }, g);
      part.lever = el("rect", { class: "lever", x: c.x - 9, width: 18, height: 18, rx: 4 }, g);
      const [dx, dy] = c.label === "below" ? [0, 52] : [46, -2];
      text(g, "code", c.x + dx, c.y + dy, c.id);
      text(g, "name", c.x + dx, c.y + dy + 20, c.name);
    } else {
      part.pad = el("circle", { class: "pad", cx: c.x, cy: c.y, r: PAD_R }, g);
      text(g, "code", c.x, c.y - 7, c.id);
      text(g, "name", c.x, c.y + 15, c.name);
    }
    this.parts.set(c.id, part);
    if (c.free) return;
    g.setAttribute("tabindex", this.interactive ? "0" : "-1");
    g.setAttribute("role", c.type === "pad" ? "button" : "slider");
    g.setAttribute("aria-label", `${c.id} ${c.name}`);
    if (c.type !== "pad") {
      g.setAttribute("aria-valuemin", "0");
      g.setAttribute("aria-valuemax", c.type === "switch" ? "2" : "100");
    }
    g.addEventListener("pointerenter", () => this.onFocus?.(c));
    g.addEventListener("focus", () => this.onFocus?.(c));
    if (this.interactive) this.bind(part);
  }

  // value: 0..1 for knobs and faders, a Position for switches, pressure for pads
  draw(id, value) {
    this.values.set(id, value);
    const { c, g, ...p } = this.parts.get(id);
    if (c.type === "knob") {
      const deg = -SWEEP / 2 + value * SWEEP;
      p.arc.setAttribute("d", value > 0.002 ? arc(c.x, c.y, KNOB_R, -SWEEP / 2, deg) : "");
      const [x, y] = polar(c.x, c.y, KNOB_R - 18, deg);
      p.dot.setAttribute("cx", x);
      p.dot.setAttribute("cy", y);
      g.setAttribute("aria-valuenow", Math.round(value * 100));
    } else if (c.type === "fader") {
      const y = FADER.bottom - value * (FADER.bottom - FADER.top);
      p.travel.setAttribute("y1", y);
      p.cap.setAttribute("y", y - 10);
      g.setAttribute("aria-valuenow", Math.round(value * 100));
    } else if (c.type === "switch") {
      const offset = { [POSITION.up]: -26, [POSITION.centre]: -9, [POSITION.down]: 8 }[value];
      p.lever.setAttribute("y", c.y + offset);
      g.setAttribute("aria-valuenow", value);
      g.setAttribute("aria-valuetext", c.free ? "Free" : END[value === POSITION.up ? 0 : value === POSITION.centre ? 1 : 2]);
    } else {
      p.pad.classList.toggle("press", value > 0);
      p.pad.style.fillOpacity = value > 0 ? 0.35 + 0.65 * value : "";
    }
  }

  highlight(id) {
    for (const [key, { g }] of this.parts) g.classList.toggle("active", key === id);
  }

  bind({ c, g }) {
    const set = (v) => this.onInput?.(c, v);
    if (c.type === "pad") {
      g.addEventListener("pointerdown", (e) => {
        e.preventDefault();
        g.setPointerCapture(e.pointerId);
        this.onPad?.(c, true, e);
      });
      const up = (e) => this.onPad?.(c, false, e);
      g.addEventListener("pointerup", up);
      g.addEventListener("pointercancel", up);
      g.addEventListener("keydown", (e) => {
        if ((e.key === " " || e.key === "Enter") && !e.repeat) {
          e.preventDefault();
          this.onPad?.(c, true, e);
        }
      });
      g.addEventListener("keyup", (e) => (e.key === " " || e.key === "Enter") && this.onPad?.(c, false, e));
      return;
    }
    if (c.type === "switch") {
      g.addEventListener("pointerdown", (e) => {
        const y = this.toPanel(e).y - c.y;
        set(y < -10 ? POSITION.up : y > 10 ? POSITION.down : POSITION.centre);
      });
      g.addEventListener("keydown", (e) => {
        const order = [POSITION.up, POSITION.centre, POSITION.down];
        const at = order.indexOf(this.value(c));
        if (e.key === "ArrowUp") set(order[Math.max(0, at - 1)]);
        else if (e.key === "ArrowDown") set(order[Math.min(2, at + 1)]);
        else return;
        e.preventDefault();
      });
      return;
    }
    // Knobs turn by dragging up and down, faders follow the pointer
    let drag = null;
    g.addEventListener("pointerdown", (e) => {
      e.preventDefault();
      g.setPointerCapture(e.pointerId);
      g.focus({ preventScroll: true });
      drag = { y: e.clientY, value: this.value(c) };
      if (c.type === "fader") set(this.faderValue(e));
    });
    g.addEventListener("pointermove", (e) => {
      if (!drag) return;
      if (c.type === "fader") return set(this.faderValue(e));
      const fine = e.shiftKey ? 0.2 : 1;
      set(clamp(drag.value + ((drag.y - e.clientY) / 200) * fine));
    });
    const end = () => (drag = null);
    g.addEventListener("pointerup", end);
    g.addEventListener("pointercancel", end);
    g.addEventListener("dblclick", () => c.initial !== undefined && set(c.initial));
    g.addEventListener("keydown", (e) => {
      const step = { ArrowUp: 0.01, ArrowRight: 0.01, ArrowDown: -0.01, ArrowLeft: -0.01, PageUp: 0.1, PageDown: -0.1 }[e.key];
      if (step === undefined) return;
      e.preventDefault();
      set(clamp(this.value(c) + step));
    });
  }

  // The value shown, as the page set it last
  value(c) {
    return this.values.get(c.id) ?? c.initial ?? 0;
  }

  toPanel(e) {
    const pt = new DOMPoint(e.clientX, e.clientY).matrixTransform(this.svg.getScreenCTM().inverse());
    return { x: pt.x, y: pt.y };
  }

  faderValue(e) {
    return clamp((FADER.bottom - this.toPanel(e).y) / (FADER.bottom - FADER.top));
  }
}

export const clamp = (x) => Math.min(1, Math.max(0, x));
