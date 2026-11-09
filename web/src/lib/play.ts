// Play: the controls drive the firmware, the buffer and the readout show what it
// does.

import { Audio, type Input } from "./audio";
import { byId, LEVELS, LEVELS_VALUE, padOf, POSITION, type Control, type Level, type Live } from "./controls";
import { GRAIN_FIELDS, VOICES, MAX_RECORD_S, RATE, RECORD_PAD, RESET_PAD, SHIFT_PAD, SLICE_PADS, PARAM, TAP_DEFAULT, TAP_SECONDS, VIEW, type Control as Message, type Frame, type FromWorklet } from "./engine";
import { Hourglass } from "./hourglass";
import { Preview } from "./preview";
import { GrainShape } from "./shape";
import { Waveform } from "./waveform";
import { knob, pad, slider, toggle, type Handlers, type Widget } from "./widgets";

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
// Until Start, the values come from a silent copy of the firmware on the page
const controls = new Map<string, Message>(); // the last of each control
let preview: Preview | null = null;
Preview.load().then((p) => {
  if (audio.ctx) return;
  preview = p;
  for (const m of controls.values()) p.apply(m);
  refresh();
});

function send(m: Message) {
  controls.set(`${m.type}${m.index}`, m);
  audio.send(m);
  if (!preview) return;
  preview.apply(m);
  refresh();
}

// The preview's values in the readouts
function refresh() {
  if (!preview) return;
  const { params, view } = preview.read();
  last = { type: "view", led: 0, input: 0, output: 0, incoming: new Float32Array(0), grains: new Float32Array(0), peaks: new Float32Array(0), slices: new Float32Array(0), params, view };
  grainShape.update(params);
  show();
}
const waveform = new Waveform($<HTMLCanvasElement>("buffer"));
const grainFigure = $("grain-figure");
const sound = document.querySelector<HTMLElement>(".sound")!;
const soundName = sound.querySelector<HTMLElement>(".bar .label")!;
const grainShape = new GrainShape($<HTMLCanvasElement>("grain-shape"));
const hourglass = new Hourglass($<HTMLCanvasElement>("hourglass"));
let last: Frame | null = null;
// Nothing pointed at: the readout shows how to play
const HINT = { name: "How to play", does: "Drag a control, ⌥ for fine. Double-click resets. Keys: R record, 1–7 slices, Shift." };
let focused: Control | null = null;

const parts = new Map<string, Widget>();
const positions = new Map<string, number>(); // each switch's, as set

const on: Handlers = { onInput: set, onPad: press, onFocus: focus };
const add = (w: Widget) => (parts.set(w.c.id, w), w);
// In and Out, the browser's own, beside the source: the gain a handle, the
// signal the fill
function level(l: Level) {
  const s = slider(l, $("levels"), on);
  add(s);
  return s.level;
}
const inMeter = level(LEVELS[0]);
const outMeter = level(LEVELS[1]);

// As on the board: a fader at each edge, the knobs in their cells, the
// switches between Position and Length. The pads in two rows: the board's
// upper pads, then the slices in order.
add(knob(byId("S36"), $("speed"), on));
add(knob(byId("S37"), $("density"), on));
for (const id of ["S30", "S31", "S32", "S33", "S34", "S35"]) add(knob(byId(id), $(`k-${id}`), on));

add(toggle(byId("S09"), $("s09"), [[POSITION.up, "Forward"], [POSITION.centre, "Freeze"], [POSITION.down, "Reverse"]], on));
add(toggle(byId("S07"), $("s07"), [[POSITION.up, "Oct"], [POSITION.centre, "Semi"], [POSITION.down, "Off"]], on));
SLICE_PADS.forEach((index, i) => add(pad(padOf(index), $("slices"), `${i + 1}`, on)));
const FUNCTIONS: [string, string][] = [["P10", "Rec"], ["P11", "Shift"], ["P00", "Live"], ["P01", "Reset"], ["P02", "P02"]];
for (const [id, label] of FUNCTIONS) {
  const w = add(pad(byId(id), $("functions"), label, on));
  if (w.c.free) w.root.classList.add("free");
  if (id === "P10") w.root.classList.add("rec");
}

// What a tap keeps: the board's 8 s, or the others to try
const taps = TAP_SECONDS.map((seconds) => {
  const b = document.createElement("button");
  b.type = "button";
  b.setAttribute("role", "radio");
  b.textContent = `${seconds} s`;
  b.addEventListener("click", () => tap(seconds));
  $("tap").append(b);
  return [seconds, b] as const;
});
function tap(seconds: number) {
  waveform.window = seconds;
  waveform.draw();
  for (const [s, b] of taps) b.setAttribute("aria-checked", String(s === seconds));
  send({ type: "tap", index: -1, seconds });
}

for (const w of parts.values()) {
  if (w.c.type === "pad") w.draw(0);
  else set(w.c, w.c.initial ?? 0, false);
}
tap(TAP_DEFAULT);
hint();
// Leaving the controls, nothing focused in them: the hint again
for (const panel of document.querySelectorAll(".levels, .instrument")) {
  panel.addEventListener("pointerleave", () => {
    if (!document.activeElement?.closest(".levels, .instrument")) hint();
  });
}

function set(c: Control, value: number, show = true) {
  if (c.index < 0) {
    const level = c as Level;
    parts.get(c.id)!.draw(value);
    LEVELS_VALUE.set(c.id, value);
    audio.setGain(c.id === "IN" ? "input" : "output", level.gain(value));
    parts.get(c.id)!.value?.(level.value!(new Float32Array(0), { sound: 0, recording: 0 }));
    if (show) focus(c);
    return;
  }
  parts.get(c.id)?.draw(value);
  if (c.free) return;
  if (c.type === "switch") {
    positions.set(c.id, value);
    send({ type: "switch", index: c.index, position: value });
  }
  else send({ type: "knob", index: c.index, value });
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
  if (c.free) return;
  const who = e.pointerId ?? "key";
  let t = touches.get(c.index);
  if (down) {
    if (!t) touches.set(c.index, (t = { since: performance.now(), pointers: new Set(), pen: null, c }));
    t.pointers.add(who);
    if (e.pointerType === "pen" && e.pressure) t.pen = e.pressure;
    sendPad(t);
    focus(c);
    if (c.index === SHIFT_PAD) layer();
    return;
  }
  if (!t) return;
  t.pointers.delete(who);
  if (t.pointers.size) return;
  touches.delete(c.index);
  if (c.index === SHIFT_PAD) layer();
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

// Keys: 1–7 the places, R record, T turn, S shake, M mute, H hold
const KEYS: Record<string, number> = { r: RECORD_PAD, shift: SHIFT_PAD, "0": RESET_PAD };
SLICE_PADS.forEach((index, i) => (KEYS[`${i + 1}`] = index));
function keyPad(e: KeyboardEvent) {
  const key = e.key.toLowerCase();
  return key in KEYS ? KEYS[key] : null;
}
addEventListener("keydown", (e) => {
  const index = keyPad(e);
  if (index === null || e.repeat || e.metaKey || e.ctrlKey || (e.target as Element).closest?.("input")) return;
  press(padOf(index), true, { pointerId: `key${index}` });
});
addEventListener("keyup", (e) => {
  const index = keyPad(e);
  if (index !== null) press(padOf(index), false, { pointerId: `key${index}` });
});
addEventListener("blur", () => {
  for (const t of [...touches.values()]) for (const who of [...t.pointers]) press(t.c, false, { pointerId: who });
});

// P11 held: the knobs' twins, brought forward at once
function shifted() {
  return touches.has(SHIFT_PAD);
}
function layer() {
  document.querySelector(".instrument")?.classList.toggle("shifted", shifted());
  if (focused) focus(focused);
  show();
}

// Seconds P10 has been held to record, 0 when it isn't
function recording() {
  const t = touches.get(RECORD_PAD);
  return t ? Math.min((performance.now() - t.since) / 1000, MAX_RECORD_S) : 0;
}

function hint() {
  focused = null;
  for (const w of parts.values()) w.root.classList.remove("focus");
  readout.name.textContent = HINT.name;
  readout.value.textContent = "";
  readout.does.textContent = HINT.does;
}

// The control pointed at
function focus(c: Control) {
  focused = c;
  for (const w of parts.values()) w.root.classList.toggle("focus", w.c === c);
  const twin = shifted() ? c.twin : undefined;
  readout.name.textContent = twin ? `P11 + ${c.id} ${twin.name}` : `${c.id} ${c.name}`;
  readout.does.textContent = (twin ?? c).does ?? "";
  show();
}

function show() {
  if (!last) return;
  const v: Live = { sound: last.view[VIEW.sound], recording: recording() };
  // P11 held: each knob with a twin shows the twin, its name and value.
  // Frozen by S09: Speed scrubs, and shows where the playhead is.
  const scrubbing = positions.get("S09") === POSITION.centre;
  for (const w of parts.values()) {
    if (w.c.index < 0) continue;
    const twin = shifted() ? w.c.twin : undefined;
    const scrub = !twin && scrubbing && w.c.id === "S36";
    w.value?.(twin ? twin.value(last.params) : scrub ? `${(last.view[VIEW.playhead] / RATE).toFixed(2)} s` : (w.c.value?.(last.params, v) ?? ""));
    if (w.c.twin) {
      w.label?.(twin ? (twin.short ?? twin.name) : scrub ? "Scrub" : (w.c.short ?? w.c.name));
      w.root.classList.toggle("twin", !!twin);
    }
  }
  const twin = shifted() ? focused?.twin : undefined;
  if (focused) readout.value.textContent = twin ? twin.value(last.params) : (focused.value?.(last.params, v) ?? "");
  // Live: the window behind the input, the Live pad lit while on
  const live = last.view[VIEW.live] === 1;
  sound.classList.toggle("live", live);
  soundName.textContent = live ? "Live" : v.recording > 0 || !v.sound ? "Input" : "Recording";
  parts.get("P00")?.root.classList.toggle("latched", live);
  held.textContent = v.recording ? `● ${v.recording.toFixed(1)} s` : live ? `${(v.sound / RATE).toFixed(0)} s back` : v.sound ? `${(v.sound / RATE).toFixed(1)} s` : "Empty";
  held.classList.toggle("on", v.recording > 0);
  const size = last.params[PARAM.grainMs];
  grainFigure.textContent = `${size < 1000 ? `${Math.round(size)} ms` : `${(size / 1000).toFixed(2)} s`} · ${Math.round(last.grains.length / GRAIN_FIELDS)}/${VOICES}`;
}

function onMessage(m: FromWorklet) {
  if (m.type === "view") {
    last = m;
    led.style.setProperty("--led", String(m.led));
    inMeter(m.input);
    outMeter(m.output);
    waveform.recording = recording();
    waveform.update(m);
    grainShape.update(m.params);
    hourglass.update(m);
    show();
    for (const t of touches.values()) sendPad(t);
  } else if (m.type === "error") fail("Can't start", m.message);
}

function fail(name: string, message: string) {
  readout.name.textContent = name;
  readout.value.textContent = "";
  readout.does.textContent = message;
}

// The source goes into the input: chosen before or after Start
let source: Input = "demo";

start.addEventListener("click", async () => {
  if (!audio.ctx) {
    start.disabled = true;
    try {
      await audio.start();
      await choose(source);
    } catch (err) {
      fail("Can't start", (err as Error).message);
      start.disabled = false;
      return;
    }
    waveform.started = true;
    preview = null;
    start.disabled = false;
  } else await audio.pause(audio.running);
  start.textContent = audio.running ? "Pause" : "Start";
  start.classList.toggle("on", audio.running);
});

async function choose(kind: Input) {
  if (audio.ctx) {
    try {
      await audio.use(kind);
    } catch (err) {
      fail(kind === "mic" ? "No microphone" : "Can't play this file", (err as Error).message);
      return;
    }
  }
  source = kind;
  const name = kind instanceof File ? "file" : kind;
  for (const b of inputs) b.setAttribute("aria-pressed", String(b.dataset.input === name));
}

for (const b of inputs) b.addEventListener("click", () => (b.dataset.input === "file" ? file.click() : choose(b.dataset.input as Input)));
file.addEventListener("change", () => file.files?.[0] && choose(file.files[0]));
