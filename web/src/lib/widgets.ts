// Play's controls: knobs, levers and pads, each for one entry of
// CONTROLS. They report through the callbacks and draw what they are given.

import type { Control } from "./controls";

export interface Handlers {
  onInput: (c: Control, value: number) => void;
  onPad: (c: Control, down: boolean, e: { pointerId?: number | string; pointerType?: string; pressure?: number }) => void;
  onFocus: (c: Control) => void;
}

export interface Widget {
  c: Control;
  root: HTMLElement;
  draw: (value: number) => void;
  value?: (text: string) => void;
  // The name shown, for a knob's twin while P11 is held
  label?: (text: string) => void;
  // Filled from the centre, as the twin shown may not be
  centred?: (on: boolean) => void;
}

const SVG = "http://www.w3.org/2000/svg";
const SWEEP = 270; // degrees, from 7:30 to 4:30
const R = 22;
const clamp = (x: number) => Math.min(1, Math.max(0, x));

function svg<K extends keyof SVGElementTagNameMap>(tag: K, attrs: Record<string, string | number>, parent: Element) {
  const e = document.createElementNS(SVG, tag);
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, String(v));
  parent.append(e);
  return e;
}

function html<K extends keyof HTMLElementTagNameMap>(tag: K, attrs: Record<string, string | number>, parent: Element) {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, String(v));
  parent.append(e);
  return e;
}

function polar(deg: number, r = R): [number, number] {
  const a = ((deg - 90) * Math.PI) / 180;
  return [28 + r * Math.cos(a), 28 + r * Math.sin(a)];
}

function arc(from: number, to: number) {
  const [x0, y0] = polar(from);
  const [x1, y1] = polar(to);
  return `M${x0} ${y0}A${R} ${R} 0 ${to - from > 180 ? 1 : 0} 1 ${x1} ${y1}`;
}

// A knob: label, dial, value. Drag up and down (Option: fine), arrows, double
// click for the starting value. Bipolar knobs fill from their centre. A fader
// on the board is drawn as one.
export function knob(c: Control, parent: Element, h: Handlers): Widget {
  const root = html("div", { class: c.type === "fader" ? "knob fader" : "knob", tabindex: 0, role: "slider", "aria-label": c.name, "aria-valuemin": 0, "aria-valuemax": 100 }, parent);
  const label = html("span", { class: "label" }, root);
  label.textContent = c.short ?? c.name;
  // A fader's slot stretches to its tile, its scale written beside it
  const holder = c.type === "fader" ? html("div", { class: "rail" }, root) : root;
  const dial = svg("svg", c.type === "fader" ? { viewBox: `0 0 24 ${FADER.height}`, preserveAspectRatio: "none", "aria-hidden": "true" } : { viewBox: "0 0 56 56", "aria-hidden": "true" }, holder);
  for (const s of c.type === "fader" ? (c.scale ?? []) : []) {
    const mark = html("span", { class: "mark-at" }, holder);
    mark.textContent = s.label;
    mark.style.top = `${((FADER.bottom - s.at * (FADER.bottom - FADER.top)) / FADER.height) * 100}%`;
  }
  const value = html("span", { class: "value" }, root);
  value.textContent = "–";
  let v = c.initial ?? 0;
  const mode = { centred: !!c.bipolar };
  const draw = c.type === "fader" ? faderDrawing(c, dial, mode) : dialDrawing(dial, mode);

  let drag: { y: number; v: number } | null = null;
  root.addEventListener("pointerdown", (e) => {
    e.preventDefault();
    root.setPointerCapture(e.pointerId);
    root.focus({ preventScroll: true });
    root.classList.add("dragging");
    drag = { y: e.clientY, v };
  });
  root.addEventListener("pointermove", (e) => {
    if (drag) h.onInput(c, clamp(drag.v + ((drag.y - e.clientY) / 180) * (e.altKey ? 0.2 : 1)));
  });
  const release = () => {
    drag = null;
    root.classList.remove("dragging");
  };
  root.addEventListener("pointerup", release);
  root.addEventListener("pointercancel", release);
  root.addEventListener("dblclick", () => h.onInput(c, c.initial ?? 0));
  root.addEventListener("keydown", (e) => {
    const step = ({ ArrowUp: 0.01, ArrowRight: 0.01, ArrowDown: -0.01, ArrowLeft: -0.01, PageUp: 0.1, PageDown: -0.1 } as Record<string, number>)[e.key];
    if (step === undefined) return;
    e.preventDefault();
    h.onInput(c, clamp(v + step));
  });
  root.addEventListener("pointerenter", () => h.onFocus(c));
  root.addEventListener("focus", () => h.onFocus(c));
  const show = (x: number) => {
    v = x;
    draw(x);
    root.setAttribute("aria-valuenow", String(Math.round(x * 100)));
  };
  show(v);
  const centred = (on: boolean) => {
    if (mode.centred === on) return;
    mode.centred = on;
    draw(v);
  };
  return { c, root, draw: show, value: (text) => (value.textContent = text), label: (text) => (label.textContent = text), centred };
}

// A fader: its slot, filled from the bottom or, bipolar, the centre, and a
// cap. A stop at the centre marked. It stretches to the height it is given.
const FADER = { height: 112, top: 6, bottom: 106 };
function faderDrawing(c: Control, dial: SVGSVGElement, mode: { centred: boolean }) {
  const mid = (FADER.top + FADER.bottom) / 2;
  svg("line", { class: "slot", x1: 12, x2: 12, y1: FADER.top, y2: FADER.bottom }, dial);
  for (const s of c.scale ?? []) {
    const y = FADER.bottom - s.at * (FADER.bottom - FADER.top);
    svg("line", { class: "tick-at", x1: 19, x2: 24, y1: y, y2: y }, dial);
  }
  if (c.detent) svg("line", { class: "track", x1: 4, x2: 20, y1: mid, y2: mid }, dial);
  const fill = svg("line", { class: "fill", x1: 12, x2: 12 }, dial);
  const cap = svg("rect", { class: "cap", x: 2, width: 20, height: 7, rx: 2 }, dial);
  return (x: number) => {
    const y = FADER.bottom - x * (FADER.bottom - FADER.top);
    fill.setAttribute("y1", String(mode.centred ? mid : FADER.bottom));
    fill.setAttribute("y2", String(y));
    cap.setAttribute("y", String(y - 3.5));
  };
}

// A dial: its arc, filled from the start or, bipolar, the centre, and a tick
function dialDrawing(dial: SVGSVGElement, mode: { centred: boolean }) {
  svg("path", { class: "track", d: arc(-SWEEP / 2, SWEEP / 2) }, dial);
  const fill = svg("path", { class: "fill" }, dial);
  const tick = svg("line", { class: "tick" }, dial);
  return (x: number) => {
    const deg = -SWEEP / 2 + x * SWEEP;
    const from = mode.centred ? 0 : -SWEEP / 2;
    fill.setAttribute("d", Math.abs(deg - from) > 1 ? arc(Math.min(from, deg), Math.max(from, deg)) : "");
    const [x0, y0] = polar(deg, R - 9);
    const [x1, y1] = polar(deg, R);
    tick.setAttribute("x1", String(x0));
    tick.setAttribute("y1", String(y0));
    tick.setAttribute("x2", String(x1));
    tick.setAttribute("y2", String(y1));
  };
}

// A pad: held while pressed, by pointer, or by Space and Enter
export function pad(c: Control, parent: Element, label: string, h: Handlers): Widget {
  const root = html("button", { class: "pad", type: "button", "aria-label": c.name }, parent);
  html("span", { class: "num" }, root).textContent = label;
  const sub = html("span", { class: "sub" }, root);
  root.addEventListener("pointerdown", (e) => {
    e.preventDefault();
    root.setPointerCapture(e.pointerId);
    h.onPad(c, true, e);
  });
  const up = (e: PointerEvent) => h.onPad(c, false, e);
  root.addEventListener("pointerup", up);
  root.addEventListener("pointercancel", up);
  root.addEventListener("keydown", (e) => {
    if ((e.key === " " || e.key === "Enter") && !e.repeat) {
      e.preventDefault();
      h.onPad(c, true, { pointerId: "key" });
    }
  });
  root.addEventListener("keyup", (e) => (e.key === " " || e.key === "Enter") && h.onPad(c, false, { pointerId: "key" }));
  root.addEventListener("pointerenter", () => h.onFocus(c));
  root.addEventListener("focus", () => h.onFocus(c));
  return {
    c,
    root,
    draw: (pressure) => {
      root.classList.toggle("on", pressure > 0);
      root.style.setProperty("--pressure", String(pressure));
    },
    value: (text) => (sub.textContent = text),
  };
}

// A three-way switch: its name, then its three positions stacked as the lever
// moves, the one chosen lit. Takes a knob's room: dial and value.
export function toggle(c: Control, parent: Element, options: [number, string][], h: Handlers): Widget {
  const root = html("div", { class: "toggle" }, parent);
  const group = html("div", { class: "segmented", role: "radiogroup", "aria-label": c.name }, root);
  const buttons = options.map(([position, text]): [number, HTMLButtonElement] => {
    const b = html("button", { type: "button", role: "radio" }, group);
    b.textContent = text;
    b.addEventListener("click", () => h.onInput(c, position));
    b.addEventListener("focus", () => h.onFocus(c));
    return [position, b];
  });
  root.addEventListener("pointerenter", () => h.onFocus(c));
  return {
    c,
    root,
    draw: (position) => {
      for (const [p, b] of buttons) b.setAttribute("aria-checked", String(p === position));
    },
  };
}

// A browser level: its name over a slim track, the gain its handle, the
// signal its fill. Drag across, arrows, double click for its starting value.
export function slider(c: Control, parent: Element, h: Handlers) {
  const root = html("div", { class: "slider", tabindex: 0, role: "slider", "aria-label": c.name, "aria-valuemin": 0, "aria-valuemax": 100 }, parent);
  html("span", { class: "label" }, root).textContent = c.name;
  const track = html("span", { class: "track" }, root);
  const fill = html("span", { class: "signal" }, track);
  const line = html("span", { class: "fill" }, track);
  const knobAt = html("span", { class: "handle" }, track);
  let v = c.initial ?? 0;
  const set = (e: PointerEvent) => {
    const r = track.getBoundingClientRect();
    h.onInput(c, clamp((e.clientX - r.left) / r.width));
  };
  let dragging = false;
  root.addEventListener("pointerdown", (e) => {
    e.preventDefault();
    root.setPointerCapture(e.pointerId);
    root.focus({ preventScroll: true });
    dragging = true;
    set(e);
  });
  root.addEventListener("pointermove", (e) => dragging && set(e));
  root.addEventListener("pointerup", () => (dragging = false));
  root.addEventListener("dblclick", () => h.onInput(c, c.initial ?? 0));
  root.addEventListener("keydown", (e) => {
    const step = ({ ArrowRight: 0.02, ArrowUp: 0.02, ArrowLeft: -0.02, ArrowDown: -0.02 } as Record<string, number>)[e.key];
    if (step === undefined) return;
    e.preventDefault();
    h.onInput(c, clamp(v + step));
  });
  root.addEventListener("pointerenter", () => h.onFocus(c));
  root.addEventListener("focus", () => h.onFocus(c));
  let shown = 0;
  return {
    c,
    root,
    draw: (x: number) => {
      v = x;
      knobAt.style.left = `${x * 100}%`;
      line.style.width = `${x * 100}%`;
      root.setAttribute("aria-valuenow", String(Math.round(x * 100)));
    },
    // Peak 0..1: the fill, -48 to 0 dBFS, falling back slowly
    level: (peak: number) => {
      shown = Math.max(peak, shown * 0.85);
      const db = 20 * Math.log10(Math.max(shown, 1e-6));
      fill.style.width = `${Math.max(0, Math.min(1, (db + 48) / 48)) * 100}%`;
      fill.classList.toggle("clip", db > -0.5);
    },
  };
}
