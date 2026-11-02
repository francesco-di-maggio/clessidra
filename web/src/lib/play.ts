// Play: the controls drive the firmware, the buffer and the readout show what it
// does.

import { Audio, type Input } from "./audio";
import { byId, CONTROLS, LEVELS, LEVELS_VALUE, padOf, POSITION, type Control, type Level, type Live } from "./controls";
import { FX_PAD, LAYER, MAX_RECORD_S, SHIFT_PAD, RATE, RECORD_PAD, SLICE_PADS, VIEW, type Control as Message, type Frame, type FromWorklet } from "./engine";
import { GrainView } from "./grain";
import { Incoming } from "./incoming";
import { Preview } from "./preview";
import { Waveform } from "./waveform";
import { knob, meter, pad, segmented, type Handlers, type Widget } from "./widgets";

const LIGHT = 0.35; // a touch, as the board's pads read it (0.3–0.5)
const FIRM_S = 1.2; // held this long, a pad reaches full pressure
const MIN_TOUCH_MS = 20; // shorter touches are released late, so a block sees them
const LATCH_MS = 300; // a layer pad let go sooner stays on, as a latch
const LAYER_PADS = new Map<number, number>([[SHIFT_PAD, LAYER.shift], [FX_PAD, LAYER.fx]]);

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
const HINT = { name: "Play", does: "Drag a knob · ⇧ fine · Double-click resets · Tap Shift or FX to latch · R records · 1–7 slices" };
let focused: Control | null = null;
const parts = new Map<string, Widget>();

const on: Handlers = { onInput: set, onPad: press, onFocus: focus };
const add = (w: Widget) => parts.set(w.c.id, w);
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

// The layer the pads hold, as the firmware picks it: FX over Shift. Each
// control's value as last set, its own or on a layer.
let layer = 0;
const values = new Map<string, number>();

// Controls in the order the sound goes: playhead, grain, FX, mix. Each
// randomness small, below its own. A fader takes both rows.
function cell(parent: HTMLElement, below = false) {
  const e = document.createElement("div");
  e.className = below ? "cell below" : "cell";
  parent.append(e);
  return e;
}
const COLUMNS: [string, string[]][] = [
  ["playhead", ["S30", "S35", "S36"]],
  ["grain", ["S34", "S31", "S32", "S33"]],
  ["fx", ["P00+S31", "P00+S32"]],
  ["mix", ["S37"]],
];
for (const [cluster, ids] of COLUMNS) {
  for (const id of ids) {
    const c = byId(id);
    const at = cell($(cluster));
    add(knob(c, at, on));
    if (c.layer === LAYER.fx) parts.get(id)!.root.classList.add("fx");
    for (const l of Object.values(LAYER)) if (onLayer(c, l)) parts.get(id)!.root.classList.add(`on${l}`);
    if (c.type === "fader") {
      at.classList.add("tall");
      continue;
    }
    const random = CONTROLS.find((r) => r.id === `P11+${id}`);
    const below = cell($(cluster), true);
    if (!random) continue;
    add(knob(random, below, on));
    parts.get(random.id)!.root.classList.add("random");
  }
}

// Switches, then the pads: record, the layers, P01 and restart above, the
// slices below
add(segmented(byId("S09"), $("switches"), [[POSITION.up, "Fwd"], [POSITION.centre, "Frz"], [POSITION.down, "Rev"]], on));
add(segmented(byId("S07"), $("switches"), [[POSITION.up, "Once"], [POSITION.centre, "Gate"], [POSITION.down, "Loop"]], on));
add(pad(byId("P10"), $("functions"), "Rec", on));
add(pad(byId("P11"), $("functions"), "Shift", on));
add(pad(byId("P00"), $("functions"), "FX", on));
add(pad(byId("P01"), $("functions"), "P01", on));
(parts.get("P01")!.root as HTMLButtonElement).disabled = true;
add(pad(byId("P02"), $("functions"), "↺", on));
SLICE_PADS.forEach((index, i) => add(pad(padOf(index), $("slices"), `${i + 1}`, on)));

for (const w of parts.values()) {
  if (w.c.type === "pad") w.draw(0);
  else set(w.c, w.c.initial ?? 0, false);
  // and each knob's value on the layers it has none on its own widget
  for (const l of Object.values(LAYER)) {
    const on = onLayer(w.c, l);
    if (on && !parts.has(on.id)) set(on, on.initial ?? 0, false);
  }
}
hint();
// Leaving the controls, nothing focused in them: the hint again
for (const panel of document.querySelectorAll(".levels, .strip, .pads")) {
  panel.addEventListener("pointerleave", () => {
    if (!document.activeElement?.closest(".levels, .strip, .pads")) hint();
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
  // A layer held: a knob with a function on it turns that, as on the board
  const target = onLayer(c, layer) ?? c;
  values.set(target.id, value);
  parts.get(target.id)?.draw(value);
  if (target.layer && target.layer === layer) {
    // The firmware reads the knob on the layer held: it starts where the value
    // is, so the pick-up takes it at once
    parts.get(ownOf(target).id)!.draw(value);
    send({ type: "knob", index: target.index, value });
  } else if (target.layer) send({ type: "layer", layer: target.layer, index: target.index, value });
  else if (c.type === "switch") send({ type: "switch", index: c.index, position: value });
  else send({ type: "knob", index: c.index, value });
  if (show) focus(c);
}

const ownOf = (l: Control) => CONTROLS.find((c) => !c.layer && (c.type === "knob" || c.type === "fader") && c.index === l.index)!;

// The pads hold a layer, or none. Each knob then reads, and shows, the value
// on it, so the firmware's pick-up takes it without a jump.
function setLayer(next: number) {
  if (next === layer) return;
  layer = next;
  send({ type: "pad", index: SHIFT_PAD, touched: layer === LAYER.shift, pressure: layer === LAYER.shift ? 1 : 0 });
  send({ type: "pad", index: FX_PAD, touched: layer === LAYER.fx, pressure: layer === LAYER.fx ? 1 : 0 });
  for (const w of parts.values()) {
    const on = onLayer(w.c, layer);
    if (!onLayer(w.c, LAYER.shift) && !onLayer(w.c, LAYER.fx)) continue;
    const value = values.get((on ?? w.c).id) ?? (on ?? w.c).initial ?? 0;
    w.as?.(on ?? w.c);
    w.draw(value);
    send({ type: "knob", index: w.c.index, value });
  }
  for (const [pad, l] of LAYER_PADS) parts.get(padOf(pad).id)!.draw(layer === l ? 0.6 : 0);
  document.body.classList.toggle("layer-shift", layer === LAYER.shift);
  document.body.classList.toggle("layer-fx", layer === LAYER.fx);
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
  if (LAYER_PADS.has(c.index)) return pressLayer(c, down);
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

// Keys: R records, 1–7 trigger the slices, first to last
function keyPad(e: KeyboardEvent) {
  if (e.key === "r" || e.key === "R") return RECORD_PAD;
  const n = Number(e.key);
  return n >= 1 && n <= 7 ? SLICE_PADS[n - 1] : null;
}
addEventListener("keydown", (e) => {
  if (e.key === "Escape") setLayer(0);
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
  for (const index of [...layerPress.keys()]) pressLayer(padOf(index), false);
});

// Seconds P10 has been held, 0 when it isn't
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

// The control pointed at, or on the layer held, its function there
function focus(c: Control) {
  focused = c;
  for (const w of parts.values()) w.root.classList.toggle("focus", w.c === c);
  const shown = onLayer(c, layer) ?? c;
  readout.name.textContent = `${shown.board ?? shown.id} ${shown.name}`;
  readout.does.textContent = shown.does ?? "";
  show();
}

function show() {
  if (!last) return;
  const triggers = SLICE_PADS.map((_, i) => last!.view[VIEW.triggers + i] >= 0);
  const slices = SLICE_PADS.map((_, i) => last!.view[VIEW.edges + i + 1] - last!.view[VIEW.edges + i]);
  const v: Live = { sound: last.view[VIEW.sound], triggers, slices, recording: recording() };
  const shown = (c: Control) => onLayer(c, layer) ?? c;
  for (const w of parts.values()) if (w.c.index >= 0) w.value?.(shown(w.c).value?.(last.params, v) ?? "");
  if (focused) readout.value.textContent = shown(focused).value?.(last.params, v) ?? "";
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
