// Play: the controls drive the firmware, the sand and the readout show what it
// does.

import { Audio, type Input } from "./audio";
import { byId, LEVELS, LEVELS_VALUE, padOf, POSITION, type Control, type Level, type Live } from "./controls";
import { MAX_RECORD_S, RATE, RECORD_PAD, SLICE_PADS, VIEW, type Frame, type FromWorklet } from "./engine";
import { Sand } from "./sand";
import { knob, meter, pad, segmented, type Handlers, type Widget } from "./widgets";

const LIGHT = 0.35; // a touch, as the board's pads read it (0.3–0.5)
const FIRM_S = 1.2; // held this long, a pad reaches full pressure
const MIN_TOUCH_MS = 20; // shorter touches are released late, so a block sees them

const $ = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
const start = $<HTMLButtonElement>("start");
const inputs = [...document.querySelectorAll<HTMLButtonElement>("#input button")];
const file = $<HTMLInputElement>("file");
const led = $("led");
const held = $("held");
const readout = { name: $("readout-name"), value: $("readout-value"), does: $("readout-does") };

const audio = new Audio(onMessage);
const sand = new Sand($<HTMLCanvasElement>("sand"));
let last: Frame | null = null;
let focused = byId("P11");
const parts = new Map<string, Widget>();

const on: Handlers = { onInput: set, onPad: press, onFocus: focus };
const add = (w: Widget) => parts.set(w.c.id, w);
add(pad(byId("P11"), $("record"), "Rec", on));
add(knob(byId("S36"), $("time"), on));
add(segmented(byId("S07"), $("time"), [[POSITION.up, "One-shot"], [POSITION.centre, "Turn"], [POSITION.down, "Loop"]], on));
for (const id of ["S30", "S35"]) add(knob(byId(id), $("selection"), on));
for (const id of ["S37", "S31", "S32", "S33"]) add(knob(byId(id), $("grain"), on));
const meters = LEVELS.map((l) => {
  const cell = document.createElement("div");
  cell.className = "level";
  $("levels").append(cell);
  add(knob(l, cell, on));
  return meter(cell, l.name);
});
SLICE_PADS.forEach((index, i) => add(pad(padOf(index), $("slices"), `${i + 1}`, on)));

for (const w of parts.values()) {
  if (w.c.type === "pad") w.draw(0);
  else set(w.c, w.c.initial ?? 0, false);
}
focus(focused);

function set(c: Control, value: number, show = true) {
  const w = parts.get(c.id)!;
  w.draw(value);
  if (c.index < 0) {
    const level = c as Level;
    LEVELS_VALUE.set(c.id, value);
    audio.setGain(c.id === "IN" ? "input" : "output", level.gain(value));
    w.value?.(level.value!(new Float32Array(0), { sound: 0, slice: 0, recording: 0 }));
    if (show) focus(c);
    return;
  }
  if (c.type === "switch") audio.send({ type: "switch", index: c.index, position: value });
  else audio.send({ type: "knob", index: c.index, value });
  if (show) focus(c);
}

// Pads: a pen's pressure where there is one, else a touch that firms up while held
interface Touch {
  since: number;
  pointers: Set<number | string>;
  pen: number | null;
  c: Control;
}
const touches = new Map<number, Touch>(); // by pad index

function press(c: Control, down: boolean, e: { pointerId?: number | string; pointerType?: string; pressure?: number }) {
  const who = e.pointerId ?? "key";
  let t = touches.get(c.index);
  if (down) {
    if (!t) touches.set(c.index, (t = { since: performance.now(), pointers: new Set(), pen: null, c }));
    t.pointers.add(who);
    if (e.pointerType === "pen" && e.pressure) t.pen = e.pressure;
    sendPad(t);
    focus(c);
    return;
  }
  if (!t) return;
  t.pointers.delete(who);
  if (t.pointers.size) return;
  touches.delete(c.index);
  const off = () => {
    if (touches.has(c.index)) return;
    audio.send({ type: "pad", index: c.index, touched: false, pressure: 0 });
    parts.get(c.id)!.draw(0);
  };
  const late = MIN_TOUCH_MS - (performance.now() - t.since);
  if (late > 0) setTimeout(off, late);
  else off();
}

function sendPad(t: Touch) {
  const p = t.pen ?? Math.min(1, LIGHT + ((performance.now() - t.since) / 1000 / FIRM_S) * (1 - LIGHT));
  audio.send({ type: "pad", index: t.c.index, touched: true, pressure: p });
  parts.get(t.c.id)!.draw(p);
}

addEventListener("pointermove", (e) => {
  if (e.pointerType !== "pen") return;
  for (const t of touches.values()) if (t.pointers.has(e.pointerId)) t.pen = e.pressure;
});

// Keys: R records, 1–7 play the slices, first to last
function keyPad(e: KeyboardEvent) {
  if (e.key === "r" || e.key === "R") return RECORD_PAD;
  const n = Number(e.key);
  return n >= 1 && n <= 7 ? SLICE_PADS[n - 1] : null;
}
addEventListener("keydown", (e) => {
  const index = keyPad(e);
  if (index === null || e.repeat || e.metaKey || e.ctrlKey || (e.target as Element).closest?.("input")) return;
  press(padOf(index), true, { pointerId: `key${e.key}` });
});
addEventListener("keyup", (e) => {
  const index = keyPad(e);
  if (index !== null) press(padOf(index), false, { pointerId: `key${e.key}` });
});
addEventListener("blur", () => {
  for (const t of [...touches.values()]) for (const who of [...t.pointers]) press(t.c, false, { pointerId: who });
});

// Seconds P11 has been held, 0 when it isn't
function recording() {
  const t = touches.get(RECORD_PAD);
  return t ? Math.min((performance.now() - t.since) / 1000, MAX_RECORD_S) : 0;
}

function focus(c: Control) {
  focused = c;
  for (const w of parts.values()) w.root.classList.toggle("focus", w.c === c);
  readout.name.textContent = `${c.id} ${c.name}`;
  readout.does.textContent = c.does ?? "";
  show();
}

function show() {
  if (!last) return;
  const v: Live = { sound: last.view[VIEW.sound], slice: last.view[VIEW.slice], recording: recording() };
  for (const w of parts.values()) if (w.c.index >= 0) w.value?.(w.c.value?.(last.params, v) ?? "");
  readout.value.textContent = focused.value?.(last.params, v) ?? "";
  held.textContent = v.recording ? `● ${v.recording.toFixed(1)} s` : v.sound ? `${(v.sound / RATE).toFixed(1)} s` : "Empty";
  held.classList.toggle("on", v.recording > 0);
  for (const w of parts.values()) if (w.c.slice !== undefined) w.root.classList.toggle("playing", w.c.slice === v.slice);
}

function onMessage(m: FromWorklet) {
  if (m.type === "view") {
    last = m;
    led.style.setProperty("--led", String(m.led));
    meters[0](m.input);
    meters[1](m.output);
    sand.recording = recording();
    sand.update(m);
    show();
    for (const t of touches.values()) sendPad(t);
  } else if (m.type === "error") fail("Can't start", m.message);
}

function fail(name: string, message: string) {
  readout.name.textContent = name;
  readout.value.textContent = "";
  readout.does.textContent = message;
}

start.addEventListener("click", async () => {
  if (!audio.ctx) {
    start.disabled = true;
    try {
      await audio.start();
      await choose("demo");
    } catch (err) {
      fail("Can't start", (err as Error).message);
      start.disabled = false;
      return;
    }
    sand.started = true;
    for (const b of inputs) b.disabled = false;
    start.disabled = false;
  } else await audio.pause(audio.running);
  start.textContent = audio.running ? "Pause" : "Play";
  start.classList.toggle("on", audio.running);
});

async function choose(kind: Input) {
  try {
    await audio.use(kind);
  } catch (err) {
    fail(kind === "mic" ? "No microphone" : "Can't play this file", (err as Error).message);
    return;
  }
  const name = kind instanceof File ? "file" : kind;
  for (const b of inputs) b.setAttribute("aria-pressed", String(b.dataset.input === name));
}

for (const b of inputs) b.addEventListener("click", () => (b.dataset.input === "file" ? file.click() : choose(b.dataset.input as Input)));
file.addEventListener("change", () => file.files?.[0] && choose(file.files[0]));
