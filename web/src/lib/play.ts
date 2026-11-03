// Play: the controls drive the firmware, the buffer and the readout show what it
// does.

import { Audio, type Input } from "./audio";
import { byId, CONTROLS, LEVELS, LEVELS_VALUE, padOf, POSITION, type Control, type Level, type Live } from "./controls";
import { LAYER, MAX_RECORD_S, MUTE_PAD, SHIFT_PAD, RATE, RECORD_PAD, SLICE_PADS, TAP_DEFAULT, TAP_SECONDS, VIEW, type Control as Message, type Frame, type FromWorklet } from "./engine";
import { GrainView } from "./grain";
import { Incoming } from "./incoming";
import { Preview } from "./preview";
import { Waveform } from "./waveform";
import { knob, meter, pad, segmented, type Handlers, type Widget } from "./widgets";

const LIGHT = 0.35; // a touch, as the board's pads read it (0.3–0.5)
const FIRM_S = 1.2; // held this long, a pad reaches full pressure
const MIN_TOUCH_MS = 20; // shorter touches are released late, so a block sees them
const LATCH_MS = 300; // a layer pad let go sooner stays on, as a latch
const LAYER_PADS = new Map<number, number>([[SHIFT_PAD, LAYER.shift]]);

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
const HINT = { name: "Play", does: "Drag · ⌥ fine · Double-click resets · R records · ⇧R listens · M mutes · 1–7 slices · Esc unlatches Shift" };
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

// A knob's function on a layer, if it has one
function onLayer(c: Control, layer: number) {
  if (c.layer || c.index < 0 || (c.type !== "knob" && c.type !== "fader")) return undefined;
  return CONTROLS.find((l) => l.layer === layer && l.index === c.index);
}

// The layer Shift holds, or none. Each control's value as last set, its own or
// on the layer.
let layer = 0;
const values = new Map<string, number>();

// The rack: speed and selection, grain and filter. Each knob above, its
// function on Shift small below it; each fader with its function on Shift
// small beside it. Speed has none on Shift: it stays Speed, as on the board.
function cell(parent: HTMLElement, kind = "") {
  const e = document.createElement("div");
  e.className = `cell ${kind}`;
  parent.append(e);
  return e;
}
const RACK: [string, string[]][] = [
  ["speed", ["S36"]],
  ["selection", ["S30", "S35"]],
  ["grain", ["S31", "S32", "S33", "S34"]],
  ["filter", ["S37"]],
];
const FREE: Control = { id: "free", type: "fader", index: -2, x: 0, y: 0, name: "Free", short: "—", board: "P11 + S36", does: "No function yet: Speed stays Speed.", initial: 0.5 };
const none: Handlers = { onInput: () => {}, onPad: () => {}, onFocus: focus };
for (const [tile, ids] of RACK) {
  for (const id of ids) {
    const c = byId(id);
    const twin = onLayer(c, LAYER.shift);
    add(knob(c, cell($(tile)), on));
    if (twin) {
      add(knob(twin, cell($(tile), "twin"), on));
      continue;
    }
    const free = knob(FREE, cell($(tile), "twin"), none);
    free.root.classList.add("free");
    free.root.removeAttribute("tabindex");
    free.value?.("—");
  }
}

// The keys between the two switches: the slices white, first to last; black
// between them the pads as they sit on the board, record and shift, P00,
// P01 and restart
add(segmented(byId("S09"), $("direction"), [[POSITION.up, "Forward"], [POSITION.centre, "Freeze"], [POSITION.down, "Reverse"]], on));
add(segmented(byId("S07"), $("trigger"), [[POSITION.up, "Once"], [POSITION.centre, "Gate"], [POSITION.down, "Loop"]], on));
SLICE_PADS.forEach((index, i) => add(pad(padOf(index), $("keys"), `${i + 1}`, on)).root.classList.add("key"));
const BLACK: [string, string, number][] = [["P10", "Rec", 0], ["P11", "Shift", 1], ["P00", "Mute", 3], ["P01", "—", 4], ["P02", "Retrig", 5]];
for (const [id, label, after] of BLACK) {
  const w = add(pad(byId(id), $("keys"), label, on));
  w.root.classList.add("key", "black");
  if (id === "P10") w.root.classList.add("rec");
  w.root.style.left = `calc((100% - 24px) / 7 * ${after + 0.5} + ${after * 4 + 2}px)`;
  if (!w.c.free) continue;
  w.root.classList.add("free");
  w.root.setAttribute("aria-disabled", "true");
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
  // and each knob's value on the layers it has none on its own widget
  for (const l of Object.values(LAYER)) {
    const on = onLayer(w.c, l);
    if (on && !parts.has(on.id)) set(on, on.initial ?? 0, false);
  }
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
  // Each control sets its own value, Shift held or not: the page shows both
  // layers. The firmware's knob drives the layer held, the other is set.
  values.set(c.id, value);
  parts.get(c.id)?.draw(value);
  const held = c.layer ? c.layer === layer : layer === 0 || !onLayer(c, layer);
  if (c.type === "switch") send({ type: "switch", index: c.index, position: value });
  else if (held) send({ type: "knob", index: c.index, value });
  else send({ type: "layer", layer: c.layer ?? 0, index: c.index, value });
  if (show) focus(c);
}

// Shift holds its layer, or none. The firmware then reads each knob at its
// value there, so its pick-up takes it without a jump. The page shows both
// layers, so nothing on it changes but the pad.
function setLayer(next: number) {
  if (next === layer) return;
  layer = next;
  send({ type: "pad", index: SHIFT_PAD, touched: layer === LAYER.shift, pressure: layer === LAYER.shift ? 1 : 0 });
  for (const w of parts.values()) {
    const on = onLayer(w.c, layer);
    if (!onLayer(w.c, LAYER.shift)) continue;
    const value = values.get((on ?? w.c).id) ?? (on ?? w.c).initial ?? 0;
    send({ type: "knob", index: w.c.index, value });
  }
  for (const [pad, l] of LAYER_PADS) parts.get(padOf(pad).id)!.draw(layer === l ? 0.6 : 0);
  document.body.classList.toggle("layer-shift", layer === LAYER.shift);
  if (focused) focus(focused);
}

// A layer pad: held, the layer while it is; tapped, a latch until tapped again
const layerPress = new Map<number, { since: number; was: boolean }>();
function pressLayer(c: Control, down: boolean) {
  const l = LAYER_PADS.get(c.index)!;
  if (down) {
    if (!layerPress.has(c.index)) layerPress.set(c.index, { since: performance.now(), was: layer === l });
    setLayer(l);
    focus(c);
    return;
  }
  const t = layerPress.get(c.index);
  if (!t) return;
  layerPress.delete(c.index);
  if (layer === l && (t.was || performance.now() - t.since >= LATCH_MS)) setLayer(0);
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
  if (LAYER_PADS.has(c.index)) return pressLayer(c, down);
  const who = e.pointerId ?? "key";
  let t = touches.get(c.index);
  if (down) {
    // Rec touched with Shift on: it listens for this touch, as on the board
    if (!t && c.index === RECORD_PAD) listening = layer === LAYER.shift;
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

// Keys: R records, 1–7 trigger the slices, first to last
function keyPad(e: KeyboardEvent) {
  if (e.key.toLowerCase() === "r") return RECORD_PAD;
  if (e.key.toLowerCase() === "m") return MUTE_PAD;
  const n = Number(e.key);
  return n >= 1 && n <= 7 ? SLICE_PADS[n - 1] : null;
}
// ⇧ R: Rec with Shift, listening, Shift for that touch only. ⇧ alone does
// nothing, so the system's shortcuts keep it.
let keyShift = false;
addEventListener("keydown", (e) => {
  if (e.key === "Escape") setLayer(0);
  const index = keyPad(e);
  if (index === null || e.repeat || e.metaKey || e.ctrlKey || (e.target as Element).closest?.("input")) return;
  if (index === RECORD_PAD && e.shiftKey && layer !== LAYER.shift) {
    keyShift = true;
    setLayer(LAYER.shift);
  }
  press(padOf(index), true, { pointerId: `key${index}` });
});
addEventListener("keyup", (e) => {
  const index = keyPad(e);
  if (index === null) return;
  press(padOf(index), false, { pointerId: `key${index}` });
  if (index !== RECORD_PAD || !keyShift) return;
  keyShift = false;
  setLayer(0);
});
addEventListener("blur", () => {
  for (const t of [...touches.values()]) for (const who of [...t.pointers]) press(t.c, false, { pointerId: who });
  for (const index of [...layerPress.keys()]) pressLayer(padOf(index), false);
});

// Seconds P10 has been held to record, 0 when it isn't
let listening = false;
function recording() {
  const t = touches.get(RECORD_PAD);
  return t && !listening ? Math.min((performance.now() - t.since) / 1000, MAX_RECORD_S) : 0;
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
  readout.name.textContent = `${c.board ?? c.id} ${c.name}`;
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
  const listen = listening && touches.has(RECORD_PAD);
  held.textContent = v.recording ? `● ${v.recording.toFixed(1)} s` : listen ? "Listen" : v.sound ? `${(v.sound / RATE).toFixed(1)} s` : "Empty";
  held.classList.toggle("on", v.recording > 0 || listen);
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
