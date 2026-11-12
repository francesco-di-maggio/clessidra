// Play: the controls drive the firmware, the views and each control's value
// show what it does, its tooltip what it is for.

import { Audio, type Input } from "./audio";
import { byId, LEVELS, LEVELS_VALUE, padOf, POSITION, type Control, type Level, type State } from "./controls";
import { KNOBS, MAX_RECORD_S, RATE, RECORD_PAD, RESET_PAD, SHIFT_PAD, SLICE_PADS, VIEW, type Control as Message, type Frame, type FromWorklet } from "./engine";
import { Incoming } from "./incoming";
import { Preview } from "./preview";
import { Hourglass } from "./hourglass";
import { Waveform } from "./waveform";
import { gain, knob, pad, toggle, type Handlers, type Widget } from "./widgets";

const LIGHT = 0.35; // a touch, as the board's pads read it (0.3–0.5)
const FIRM_S = 1.2; // held this long, a pad reaches full pressure
const MIN_TOUCH_MS = 20; // shorter touches are released late, so a block sees them

const $ = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
const start = $<HTMLButtonElement>("start");
const inputs = [...document.querySelectorAll<HTMLButtonElement>("#input button")];
const file = $<HTMLInputElement>("file");

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

// The preview's values on the controls
function refresh() {
  if (!preview) return;
  const { params, values, view } = preview.read();
  last = { type: "view", led: 0, input: 0, output: 0, incoming: new Float32Array(0), incomingAt: 0, grains: new Float32Array(0), peaks: new Float32Array(0), slices: new Float32Array(0), params, values, view };
  hourglass.update(params, false);
  show();
}
const waveform = new Waveform($<HTMLCanvasElement>("buffer"));
const monitor = $("monitor");
const incoming = new Incoming($<HTMLCanvasElement>("incoming"));
const hourglass = new Hourglass($<HTMLCanvasElement>("glass"));
// The board's LED: its light output, as bright to the eye
const led = $("led");
const light = (output: number) => led.style.setProperty("--light", String(Math.cbrt(output)));
let last: Frame | null = null;

const parts = new Map<string, Widget>();
const positions = new Map<string, number>(); // each switch's, as set

const on: Handlers = { onInput: set, onDefault: toDefault, onPad: press, onFocus: focus };
const add = (w: Widget) => (parts.set(w.c.id, w), w);
// In and Out, the browser's own, beside the source: the gain a handle, the
// signal the fill
// A level's name mutes it, the setting kept
const muted = new Set<string>();
function mute(c: Control) {
  if (!muted.delete(c.id)) muted.add(c.id);
  set(c, LEVELS_VALUE.get(c.id) ?? c.initial ?? 0, false);
  return muted.has(c.id);
}
function level(l: Level) {
  const s = gain(l, $("levels"), on, mute);
  add(s);
  return s.level;
}
const outMeter = level(LEVELS[0]);

// As on the board: a fader at each edge, the knobs in their cells, the
// switches between Position and Length, the pads under them.
add(knob(byId("S36"), $("speed"), on));
add(knob(byId("S37"), $("density"), on));
for (const id of ["S30", "S31", "S32", "S33", "S34", "S35"]) add(knob(byId(id), $(`k-${id}`), on));

add(toggle(byId("S09"), $("s09"), [[POSITION.down, "REVERSE"], [POSITION.centre, "FREEZE"], [POSITION.up, "FORWARD"]], on));
add(toggle(byId("S07"), $("s07"), [[POSITION.down, "ONCE"], [POSITION.centre, "TURN"], [POSITION.up, "LOOP"]], on));
// The pads as an octave: the slices the white keys, the others the black,
// Rec and Shift on C# and D#, Stop, Reset and Trig on F#, G# and A#. A slice
// held is tinted in the recording.
SLICE_PADS.forEach((index, i) => {
  const w = add(pad(padOf(index), $("keys"), `${i + 1}`, on));
  const draw = w.draw;
  w.draw = (p) => (draw(p), waveform.hold(i, p));
});
const BLACK: [string, string, number][] = [["P10", "Rec", 1], ["P11", "Shift", 2], ["P00", "Stop", 4], ["P01", "Reset", 5], ["P02", "Trig", 6]];
for (const [id, label, edge] of BLACK) {
  const w = add(pad(byId(id), $("keys"), label, on));
  w.root.classList.add("black");
  w.root.style.setProperty("--edge", String(edge));
  if (id === "P10") w.root.classList.add("rec");
}

// Every control at its starting value, sent to the firmware
for (const w of parts.values()) {
  if (w.c.type === "pad") w.draw(0);
  else set(w.c, w.c.initial ?? 0, false);
}
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
    audio.setGain("output", muted.has(c.id) ? 0 : level.gain(value));
    parts.get(c.id)!.value?.(muted.has(c.id) ? "−∞ dB" : level.value!(new Float32Array(0), { sound: 0, recording: 0 }));
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

// A double click: the value back to its default, outright. Shift held: the
// twin's, the knob where it is.
function toDefault(c: Control) {
  const value = c.initial ?? 0;
  if (c.index < 0 || c.free) return set(c, value);
  const twin = !!c.twin && shifted();
  const m: Message = { type: "knob", index: c.index, value, reset: twin ? "twin" : "own" };
  if (twin) {
    audio.send(m);
    preview?.apply(m);
    refresh();
  } else {
    parts.get(c.id)?.draw(value);
    send(m);
  }
  focus(c);
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
    send({ type: "pad", index: c.index, touched: false, pressure: 0 });
    parts.get(c.id)!.draw(0);
  };
  const late = MIN_TOUCH_MS - (performance.now() - t.since);
  if (late > 0) setTimeout(off, late);
  else off();
}

function sendPad(t: Touch) {
  const p = t.pen ?? Math.min(1, LIGHT + ((performance.now() - t.since) / 1000 / FIRM_S) * (1 - LIGHT));
  send({ type: "pad", index: t.c.index, touched: true, pressure: p });
  parts.get(t.c.id)!.draw(p);
}

addEventListener("pointermove", (e) => {
  if (e.pointerType !== "pen") return;
  for (const t of touches.values()) if (t.pointers.has(e.pointerId)) t.pen = e.pressure;
});

// Keys: 1–7 the slices, R record, Shift the second layer, 0 reset
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
  show();
}

// Seconds P10 has been held to record, 0 when it isn't
function recording() {
  const t = touches.get(RECORD_PAD);
  return t ? Math.min((performance.now() - t.since) / 1000, MAX_RECORD_S) : 0;
}

function hint() {
  for (const w of parts.values()) w.root.classList.remove("focus");
}

// The control pointed at
function focus(c: Control) {
  for (const w of parts.values()) w.root.classList.toggle("focus", w.c === c);
  show();
}

function show() {
  if (!last) return;
  const v: State = { sound: last.view[VIEW.sound], recording: recording() };
  // P11 held: each knob with a twin shows the twin, its name and value.
  // Frozen by S09: Speed scrubs, and shows where the playhead is.
  const scrubbing = positions.get("S09") === POSITION.centre;
  for (const w of parts.values()) {
    if (w.c.index < 0) continue;
    const twin = shifted() ? w.c.twin : undefined;
    const scrub = !twin && scrubbing && w.c.id === "S36";
    w.fillTo?.(last.values[(shifted() ? KNOBS : 0) + w.c.index]);
    w.value?.(twin ? twin.value(last.params) : scrub ? `${(last.view[VIEW.playhead] / RATE).toFixed(2)} s` : (w.c.value?.(last.params, v) ?? ""));
    if (w.c.twin) {
      w.label?.(twin ? (twin.short ?? twin.name) : scrub ? "Scrub" : (w.c.short ?? w.c.name));
      w.root.classList.toggle("twin", !!twin);
      w.centred?.(twin ? !!twin.bipolar : !!w.c.bipolar);
    }
  }
  // Its length is on its scale. Named with it for screen readers and tests.
  $("buffer").setAttribute("aria-label", v.sound ? `Recording, ${(v.sound / RATE).toFixed(1)} s` : "Recording, empty");
}

function onMessage(m: FromWorklet) {
  if (m.type === "view") {
    last = m;
    light(m.led);
    outMeter(m.output);
    waveform.recording = incoming.recording = recording();
    incoming.update(m.incoming, m.incomingAt);
    waveform.update(m);
    hourglass.update(m.params, m.view[VIEW.sound] > 0);
    show();
    for (const t of touches.values()) sendPad(t);
  } else if (m.type === "error") fail("Can't start", m.message);
}

function fail(name: string, message: string) {
  monitor.textContent = `${name}: ${message}`;
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
  if (!audio.running) light(0);
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

