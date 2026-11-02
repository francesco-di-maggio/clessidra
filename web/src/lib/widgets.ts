// Play's controls: knobs, pads and segmented switches, each for one entry of
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

// A knob: label, dial, value. Drag up and down (Shift: fine), arrows, double
// click for the starting value. Bipolar knobs fill from their centre.
export function knob(c: Control, parent: Element, h: Handlers): Widget {
  const root = html("div", { class: "knob", tabindex: 0, role: "slider", "aria-label": c.name, "aria-valuemin": 0, "aria-valuemax": 100 }, parent);
  html("span", { class: "label" }, root).textContent = c.name;
  const dial = svg("svg", { viewBox: "0 0 56 56", "aria-hidden": "true" }, root);
  svg("path", { class: "track", d: arc(-SWEEP / 2, SWEEP / 2) }, dial);
  const fill = svg("path", { class: "fill" }, dial);
  const tick = svg("line", { class: "tick" }, dial);
  const value = html("span", { class: "value" }, root);
  value.textContent = "–";
  let v = c.initial ?? 0;

  function draw(x: number) {
    v = x;
    const deg = -SWEEP / 2 + x * SWEEP;
    const from = c.bipolar ? 0 : -SWEEP / 2;
    fill.setAttribute("d", Math.abs(deg - from) > 1 ? arc(Math.min(from, deg), Math.max(from, deg)) : "");
    const [x0, y0] = polar(deg, R - 9);
    const [x1, y1] = polar(deg, R);
    tick.setAttribute("x1", String(x0));
    tick.setAttribute("y1", String(y0));
    tick.setAttribute("x2", String(x1));
    tick.setAttribute("y2", String(y1));
    root.setAttribute("aria-valuenow", String(Math.round(x * 100)));
  }

  let drag: { y: number; v: number } | null = null;
  root.addEventListener("pointerdown", (e) => {
    e.preventDefault();
    root.setPointerCapture(e.pointerId);
    root.focus({ preventScroll: true });
    drag = { y: e.clientY, v };
  });
  root.addEventListener("pointermove", (e) => {
    if (drag) h.onInput(c, clamp(drag.v + ((drag.y - e.clientY) / 180) * (e.shiftKey ? 0.2 : 1)));
  });
  root.addEventListener("pointerup", () => (drag = null));
  root.addEventListener("pointercancel", () => (drag = null));
  root.addEventListener("dblclick", () => h.onInput(c, c.initial ?? 0));
  root.addEventListener("keydown", (e) => {
    const step = ({ ArrowUp: 0.01, ArrowRight: 0.01, ArrowDown: -0.01, ArrowLeft: -0.01, PageUp: 0.1, PageDown: -0.1 } as Record<string, number>)[e.key];
    if (step === undefined) return;
    e.preventDefault();
    h.onInput(c, clamp(v + step));
  });
  root.addEventListener("pointerenter", () => h.onFocus(c));
  root.addEventListener("focus", () => h.onFocus(c));
  draw(v);
  return { c, root, draw, value: (text) => (value.textContent = text) };
}

// A pad: held while pressed, by pointer, or by Space and Enter
export function pad(c: Control, parent: Element, label: string, h: Handlers): Widget {
  const root = html("button", { class: "pad", type: "button", "aria-label": c.name }, parent);
  root.textContent = label;
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
  };
}

// A three-way switch: its name over a column of buttons
export function segmented(c: Control, parent: Element, options: [number, string][], h: Handlers): Widget {
  const root = html("div", { class: "switch" }, parent);
  html("span", { class: "label" }, root).textContent = c.name;
  const group = html("div", { class: "segmented", role: "radiogroup", "aria-label": c.name }, root);
  const buttons = options.map(([position, text]): [number, HTMLButtonElement] => {
    const b = html("button", { type: "button", role: "radio" }, group);
    b.textContent = text;
    b.addEventListener("click", () => h.onInput(c, position));
    b.addEventListener("pointerenter", () => h.onFocus(c));
    b.addEventListener("focus", () => h.onFocus(c));
    return [position, b];
  });
  return {
    c,
    root,
    draw: (position) => {
      for (const [p, b] of buttons) b.setAttribute("aria-checked", String(p === position));
    },
  };
}

// A level as a column of dots, -48 to 0 dBFS, lit from the bottom. The top
// dot turns orange at full scale. Falls back slowly, as a meter does.
export function meter(parent: Element, label: string) {
  const DOTS = 12;
  const root = html("div", { class: "meter", role: "meter", "aria-label": label, "aria-valuemin": -48, "aria-valuemax": 0 }, parent);
  const dots = Array.from({ length: DOTS }, () => html("i", {}, root)).reverse();
  let shown = 0;
  return (peak: number) => {
    shown = Math.max(peak, shown * 0.85);
    const db = 20 * Math.log10(Math.max(shown, 1e-6));
    const lit = Math.round(((db + 48) / 48) * DOTS);
    dots.forEach((d, i) => {
      d.classList.toggle("on", i < lit);
      d.classList.toggle("clip", i === DOTS - 1 && db > -0.5);
    });
    root.setAttribute("aria-valuenow", String(Math.round(db)));
  };
}
