// Play: the panel drives the firmware, the view and the readout show what it
// does.

import { Audio } from "./audio.js";
import { CONTROLS, Panel, SLICE_PADS } from "./panel.js";
import { Sand } from "./sand.js";

const LIGHT = 0.35; // a touch, as the board's pads read it (0.3–0.5)
const FIRM_S = 1.2; // held this long, a pad reaches full pressure
const MIN_TOUCH_MS = 20; // shorter touches are released late, so a block sees them

const $ = (id) => document.getElementById(id);
const start = $("start");
const inputs = document.querySelectorAll("#input button");
const file = $("file");
const led = $("led");
const readout = { name: $("readout-name"), value: $("readout-value"), does: $("readout-does") };

const audio = new Audio(onMessage);
const sand = new Sand($("sand"), 8 * 48000);
let last = null; // the latest view
let focused = CONTROLS.find((c) => c.id === "S34");

const panel = new Panel($("panel"), { interactive: true, onInput: set, onPad: pad, onFocus: focus });
for (const c of CONTROLS) {
  if (c.type === "pad") panel.draw(c.id, 0);
  else set(c, c.initial, false);
}
focus(focused);

function set(c, value, show = true) {
  panel.draw(c.id, value);
  if (c.free) return;
  if (c.type === "switch") audio.send({ type: "switch", index: c.index, position: value });
  else audio.send({ type: "knob", index: c.index, value });
  if (show) focus(c);
}

// Pads: a pen's pressure where there is one, else a touch that firms up while held
const held = new Map(); // pad index -> { since, pointers, pen }
function pad(c, down, e) {
  const who = e.pointerId ?? "key";
  let h = held.get(c.index);
  if (down) {
    if (!h) held.set(c.index, (h = { since: performance.now(), pointers: new Set(), pen: null }));
    h.pointers.add(who);
    if (e.pointerType === "pen" && e.pressure > 0) h.pen = e.pressure;
    sendPad(c.index);
    focus(c);
    return;
  }
  if (!h) return;
  h.pointers.delete(who);
  if (h.pointers.size) return;
  held.delete(c.index);
  const off = () => {
    if (held.has(c.index)) return;
    audio.send({ type: "pad", index: c.index, touched: false, pressure: 0 });
    panel.draw(c.id, 0);
  };
  const late = MIN_TOUCH_MS - (performance.now() - h.since);
  late > 0 ? setTimeout(off, late) : off();
}

function pressure(h) {
  if (h.pen !== null) return h.pen;
  return Math.min(1, LIGHT + ((performance.now() - h.since) / 1000 / FIRM_S) * (1 - LIGHT));
}

function sendPad(index) {
  const p = pressure(held.get(index));
  audio.send({ type: "pad", index, touched: true, pressure: p });
  panel.draw(`P${String(index).padStart(2, "0")}`, p);
}

$("panel").addEventListener("pointermove", (e) => {
  if (e.pointerType !== "pen") return;
  for (const h of held.values()) if (h.pointers.has(e.pointerId)) h.pen = e.pressure;
});

// Keys 1–7: the slices, first to last
addEventListener("keydown", (e) => {
  const n = Number(e.key);
  if (e.repeat || e.metaKey || e.ctrlKey || !(n >= 1 && n <= 7) || e.target.closest?.("input")) return;
  pad(CONTROLS.find((c) => c.index === SLICE_PADS[n - 1] && c.type === "pad"), true, { pointerId: `key${n}` });
});
addEventListener("keyup", (e) => {
  const n = Number(e.key);
  if (n >= 1 && n <= 7) pad(CONTROLS.find((c) => c.index === SLICE_PADS[n - 1] && c.type === "pad"), false, { pointerId: `key${n}` });
});
addEventListener("blur", () => {
  for (const [index, h] of held) for (const who of [...h.pointers]) pad(CONTROLS.find((c) => c.index === index && c.type === "pad"), false, { pointerId: who });
});

function focus(c) {
  if (!c || c.free) return;
  focused = c;
  panel.highlight(c.id);
  readout.name.textContent = `${c.id} · ${c.name}`;
  readout.does.textContent = c.does;
  showValue();
}

function showValue() {
  if (!last) return (readout.value.textContent = "");
  const v = { sound: last.view[2], slice: last.view[9] };
  readout.value.textContent = focused.value(last.params, v);
}

function onMessage(m) {
  if (m.type === "ready") {
    sand.samples = m.samples;
  } else if (m.type === "view") {
    last = m;
    led.style.opacity = m.led;
    sand.update(m);
    showValue();
    for (const index of held.keys()) sendPad(index);
  } else if (m.type === "error") {
    readout.name.textContent = "Can't start";
    readout.does.textContent = m.message;
  }
}

start.addEventListener("click", async () => {
  if (!audio.ctx) {
    start.disabled = true;
    try {
      await audio.start();
      await choose("demo");
    } catch (err) {
      readout.name.textContent = "Can't start";
      readout.does.textContent = err.message;
      return;
    }
    for (const b of inputs) b.disabled = false;
    start.disabled = false;
  } else await audio.pause(audio.running);
  start.querySelector("span").textContent = audio.running ? "Pause" : "Play";
});

async function choose(kind) {
  try {
    await audio.use(kind);
  } catch (err) {
    readout.name.textContent = kind === "mic" ? "No microphone" : "Can't play this file";
    readout.does.textContent = err.message;
    return;
  }
  const name = kind instanceof File ? "file" : kind;
  for (const b of inputs) b.setAttribute("aria-pressed", b.dataset.input === name);
}

for (const b of inputs) {
  b.addEventListener("click", () => (b.dataset.input === "file" ? file.click() : choose(b.dataset.input)));
}
file.addEventListener("change", () => file.files[0] && choose(file.files[0]));
