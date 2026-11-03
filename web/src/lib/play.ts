// Play: the controls drive the firmware, the buffer and the readout show what it
// does.

import { Audio, type Input } from "./audio";
import { byId, LEVELS, LEVELS_VALUE, padOf, POSITION, type Control, type Level, type Live } from "./controls";
import { DROP_PAD, MAX_RECORD_S, MUTE_PAD, RATE, RECORD_PAD, RESTART_PAD, SHAKE_PAD, SLICE_PADS, TAP_DEFAULT, TAP_SECONDS, VIEW, type Control as Message, type Frame, type FromWorklet } from "./engine";
import { GrainView } from "./grain";
import { Incoming } from "./incoming";
import { Preview } from "./preview";
import { Waveform } from "./waveform";
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
  last = { type: "view", led: 0, input: 0, output: 0, incoming: new Float32Array(0), grains: new Float32Array(0), peaks: new Float32Array(0), params, view };
  grainView.update(params);
  show();
}
const waveform = new Waveform($<HTMLCanvasElement>("buffer"));
const incoming = new Incoming($<HTMLCanvasElement>("incoming"));
const grainView = new GrainView($<HTMLCanvasElement>("grain-view"));
let last: Frame | null = null;
// Nothing pointed at: the readout shows how to play
const HINT = { name: "How to play", does: "Drag a control · ⌥ fine · Double-click resets · R record · S shake · D drop · M mute · 0 restart · 1–7 slices" };
let focused: Control | null = null;
const parts = new Map<string, Widget>();

const on: Handlers = { onInput: set, onPad: press, onFocus: focus };
const add = (w: Widget) => (parts.set(w.c.id, w), w);
// In and Out, the browser's own, beside the source
function level(l: Level) {
  const at = document.createElement("div");
  at.className = "level";
  $("levels").append(at);
  add(knob(l, at, on));
  return meter(at, l.name);
}
const inMeter = level(LEVELS[0]);
const outMeter = level(LEVELS[1]);

// The rack: Speed, the sand and the grain, Flow. The deck: the switches each
// side of the keys, white the slices first to last, black between them the
// pads as they sit on the board.
const RACK: [string, string[]][] = [
  ["speed", ["S36"]],
  ["selection", ["S30", "S35"]],
  ["grain", ["S31", "S32", "S33", "S34"]],
  ["flow", ["S37"]],
];
for (const [tile, ids] of RACK) {
  for (const id of ids) {
    const cell = document.createElement("div");
    cell.className = "cell";
    $(tile).append(cell);
    add(knob(byId(id), cell, on));
  }
}

add(segmented(byId("S09"), $("direction"), [[POSITION.up, "Forward"], [POSITION.centre, "Hold"], [POSITION.down, "Reverse"]], on));
add(segmented(byId("S07"), $("end"), [[POSITION.up, "Once"], [POSITION.centre, "Turn"], [POSITION.down, "Loop"]], on));
SLICE_PADS.forEach((index, i) => add(pad(padOf(index), $("keys"), `${i + 1}`, on)).root.classList.add("key"));
const BLACK: [string, string, number][] = [["P10", "Rec", 0], ["P11", "Shake", 1], ["P00", "Mute", 3], ["P01", "Drop", 4], ["P02", "Restart", 5]];
for (const [id, label, after] of BLACK) {
  const w = add(pad(byId(id), $("keys"), label, on));
  w.root.classList.add("key", "black");
  if (id === "P10") w.root.classList.add("rec");
  w.root.style.left = `calc((100% - 24px) / 7 * ${after + 0.5} + ${after * 4 + 2}px)`;
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
  incoming.seconds = seconds;
  incoming.draw();
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
for (const panel of document.querySelectorAll(".levels, .rack, .deck")) {
  panel.addEventListener("pointerleave", () => {
    if (!document.activeElement?.closest(".levels, .rack, .deck")) hint();
  });
}

function set(c: Control, value: number, show = true) {
  if (c.index < 0) {
    const level = c as Level;
    parts.get(c.id)!.draw(value);
    LEVELS_VALUE.set(c.id, value);
    audio.setGain(c.id === "IN" ? "input" : "output", level.gain(value));
    parts.get(c.id)!.value?.(level.value!(new Float32Array(0), { sound: 0, triggers: [], slices: [], recording: 0 }));
    if (show) focus(c);
    return;
  }
  parts.get(c.id)?.draw(value);
  if (c.type === "switch") send({ type: "switch", index: c.index, position: value });
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

// Keys: R record, S shake, D drop, M mute, 0 restart, 1–7 the slices, first to
// last
const KEYS: Record<string, number> = { r: RECORD_PAD, s: SHAKE_PAD, d: DROP_PAD, m: MUTE_PAD, "0": RESTART_PAD };
function keyPad(e: KeyboardEvent) {
  const key = e.key.toLowerCase();
  if (key in KEYS) return KEYS[key];
  const n = Number(e.key);
  return n >= 1 && n <= 7 ? SLICE_PADS[n - 1] : null;
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
  readout.name.textContent = `${c.id} ${c.name}`;
  readout.does.textContent = c.does ?? "";
  show();
}

function show() {
  if (!last) return;
  const triggers = SLICE_PADS.map((_, i) => last!.view[VIEW.triggers + i] >= 0);
  const slices = SLICE_PADS.map((_, i) => last!.view[VIEW.edges + i + 1] - last!.view[VIEW.edges + i]);
  const v: Live = { sound: last.view[VIEW.sound], triggers, slices, recording: recording() };
  for (const w of parts.values()) if (w.c.index >= 0) w.value?.(w.c.value?.(last.params, v) ?? "");
  if (focused) readout.value.textContent = focused.value?.(last.params, v) ?? "";
  held.textContent = v.recording ? `● ${v.recording.toFixed(1)} s` : v.sound ? `${(v.sound / RATE).toFixed(1)} s` : "Empty";
  held.classList.toggle("on", v.recording > 0);
  for (const w of parts.values()) if (w.c.slice !== undefined) w.root.classList.toggle("playing", triggers[w.c.slice]);
}

function onMessage(m: FromWorklet) {
  if (m.type === "view") {
    last = m;
    led.style.setProperty("--led", String(m.led));
    inMeter(m.input);
    outMeter(m.output);
    waveform.recording = incoming.recording = recording();
    waveform.update(m);
    incoming.update(m.incoming);
    grainView.update(m.params);
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
