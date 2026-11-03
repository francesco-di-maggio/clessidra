// The Play page in headless Chrome: Start, record with R, and the firmware
// answers. Serves web/dist itself. From the repo: make test, after make web.
// Chrome from $CHROME, or where it usually is.
import { spawn } from "node:child_process";
import { existsSync, mkdtempSync, readFileSync } from "node:fs";
import { createServer } from "node:http";
import { tmpdir } from "node:os";
import { extname, join } from "node:path";

const PREVIEW = 4329;
const DEBUG = 9339;
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

const chrome = [process.env.CHROME, "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome", "/usr/bin/google-chrome", "/usr/bin/chromium"].find((p) => p && existsSync(p));
if (!chrome) {
  console.log("skipped, no Chrome");
  process.exit(0);
}

let failed = 0;
function check(ok, what, value) {
  console.log(`${ok ? "ok  " : "FAIL"} ${what}${value === undefined ? "" : ` (${value})`}`);
  if (!ok) failed++;
}

// web/dist at /clessidra/, as GitHub Pages serves it
const dist = new URL("../web/dist/", import.meta.url).pathname;
const TYPES = { ".html": "text/html", ".js": "text/javascript", ".css": "text/css", ".wasm": "application/wasm", ".svg": "image/svg+xml", ".png": "image/png" };
const server = createServer((req, res) => {
  let path = decodeURIComponent(new URL(req.url, "http://x").pathname).replace(/^\/clessidra\//, "");
  if (path === "" || path.endsWith("/")) path += "index.html";
  try {
    const body = readFileSync(join(dist, path));
    res.writeHead(200, { "Content-Type": TYPES[extname(path)] ?? "application/octet-stream" });
    res.end(body);
  } catch {
    res.writeHead(404);
    res.end();
  }
}).listen(PREVIEW);
const browser = spawn(chrome, ["--headless=new", `--remote-debugging-port=${DEBUG}`, "--autoplay-policy=no-user-gesture-required", "--mute-audio", `--user-data-dir=${mkdtempSync(join(tmpdir(), "clessidra-"))}`, "about:blank"], { stdio: "ignore" });

async function waitFor(url) {
  for (let i = 0; i < 80; i++) {
    try {
      const r = await fetch(url);
      if (r.ok) return r;
    } catch {}
    await sleep(250);
  }
  throw new Error(`no answer from ${url}`);
}

try {
  const page = `http://localhost:${PREVIEW}/clessidra/play/`;
  await waitFor(page);
  const targets = await (await waitFor(`http://127.0.0.1:${DEBUG}/json`)).json();
  const ws = new WebSocket(targets.find((t) => t.type === "page").webSocketDebuggerUrl);
  await new Promise((r) => ws.addEventListener("open", r));
  let id = 0;
  const pending = new Map();
  const errors = [];
  const send = (method, params = {}) =>
    new Promise((resolve) => {
      pending.set(++id, resolve);
      ws.send(JSON.stringify({ id, method, params }));
    });
  ws.addEventListener("message", (e) => {
    const m = JSON.parse(e.data);
    if (pending.has(m.id)) pending.get(m.id)(m.result);
    if (m.method === "Runtime.exceptionThrown") errors.push(m.params.exceptionDetails.exception?.description ?? m.params.exceptionDetails.text);
  });
  const run = async (expression) => (await send("Runtime.evaluate", { expression, awaitPromise: true, returnByValue: true })).result?.value;

  await send("Runtime.enable");
  await send("Page.enable");
  await send("Page.navigate", { url: page });
  await sleep(2000);
  check(await run(`[...document.querySelectorAll("#input button")].every((b) => !b.disabled)`), "sources to choose before Start");
  await run(`document.getElementById("start").click()`);
  await sleep(1500);
  check((await run(`document.getElementById("start").textContent`)) === "Pause", "Start: running");
  await run(`dispatchEvent(new KeyboardEvent("keydown", { key: "r" }))`);
  await sleep(2000);
  await run(`dispatchEvent(new KeyboardEvent("keyup", { key: "r" }))`);
  await sleep(1500);
  const held = await run(`document.getElementById("held").textContent`);
  check(/^2\.\d s$/.test(held), "R held 2 s: recorded", held);
  const values = await run(`[...document.querySelectorAll(".knob:not(.free) .value")].map((v) => v.textContent)`);
  check(values.length > 0 && values.every((v) => v && v !== "–"), "every knob shows the firmware's value", values.join(" "));

  // Shift tapped: latched, Size turns its randomness. Tapped again: off.
  // Held past the latch time: off on release. Latched, Rec listens.
  const pad = (name, down) => run(`document.querySelector('#keys .pad[aria-label="${name}"]').dispatchEvent(new KeyboardEvent("${down ? "keydown" : "keyup"}", { key: " " }))`);
  const value = (name) => run(`[...document.querySelectorAll(".knob")].find((k) => k.getAttribute("aria-label") === "${name}").querySelector(".value").textContent`);
  const layer = () => run(`document.body.className`);
  const size = await value("Size");
  await pad("Shift", true);
  await pad("Shift", false);
  check((await layer()).includes("layer-shift"), "Shift tapped: latched", await layer());
  await run(`(() => { const k = [...document.querySelectorAll(".knob")].find((k) => k.getAttribute("aria-label") === "Size Random"); for (let i = 0; i < 30; i++) k.dispatchEvent(new KeyboardEvent("keydown", { key: "ArrowUp" })); })()`);
  await sleep(300);
  const random = await value("Size Random");
  check(random !== "Off" && (await value("Size")) === size, "Shift latched: Size Random turns, Size stays", random);
  await run(`(() => { const k = [...document.querySelectorAll(".knob")].find((k) => k.getAttribute("aria-label") === "Size"); for (let i = 0; i < 20; i++) k.dispatchEvent(new KeyboardEvent("keydown", { key: "ArrowUp" })); })()`);
  await sleep(300);
  const turned = await value("Size");
  check(turned !== size && (await value("Size Random")) === random, "Shift latched: Size still turns Size, Size Random stays", turned);
  await run(`(() => { const k = [...document.querySelectorAll(".knob")].find((k) => k.getAttribute("aria-label") === "Size"); for (let i = 0; i < 20; i++) k.dispatchEvent(new KeyboardEvent("keydown", { key: "ArrowDown" })); })()`);
  await sleep(300);
  await pad("Shift", true);
  await pad("Shift", false);
  await sleep(300);
  check(!(await layer()).includes("layer-shift") && (await value("Size")) === size, "Shift tapped again: off, Size as it was", await value("Size"));
  await pad("Shift", true);
  await sleep(500);
  await pad("Shift", false);
  check(!(await layer()).includes("layer-shift"), "Shift held: off on release", await layer());
  await run(`dispatchEvent(new KeyboardEvent("keydown", { key: "Shift", shiftKey: true }))`);
  const shiftAlone = await layer();
  await run(`dispatchEvent(new KeyboardEvent("keyup", { key: "Shift" }))`);
  check(!shiftAlone.includes("layer-shift"), "⇧ alone: nothing, the system keeps it", shiftAlone);
  const before = await run(`document.getElementById("held").textContent`);
  await run(`dispatchEvent(new KeyboardEvent("keydown", { key: "R", shiftKey: true }))`);
  await sleep(400);
  const keyListen = await run(`document.getElementById("held").textContent`);
  await run(`dispatchEvent(new KeyboardEvent("keyup", { key: "r" }))`);
  await sleep(300);
  check(keyListen === "Listen" && !(await layer()).includes("layer-shift") && (await run(`document.getElementById("held").textContent`)) === before, "⇧R: listens, nothing recorded, Shift off after", keyListen);

  await pad("Shift", true);
  await pad("Shift", false);
  await pad("Record", true);
  await sleep(500);
  const listen = await run(`document.getElementById("held").textContent`);
  await pad("Record", false);
  await sleep(1500);
  check(listen === "Listen" && (await run(`document.getElementById("held").textContent`)) === held, "Shift + Rec: listens, nothing recorded", listen);
  await pad("Shift", true);
  await pad("Shift", false);

  // A tap keeps the seconds set over the input
  await run(`[...document.querySelectorAll("#tap button")].find((b) => b.textContent === "4 s").click()`);
  await pad("Record", true);
  await sleep(100);
  await pad("Record", false);
  await sleep(1500);
  const tapped = await run(`document.getElementById("held").textContent`);
  check(tapped === "4.0 s", "4 s set, Rec tapped: the last 4 s", tapped);
  await run(`document.querySelector('#keys .pad[aria-label="Free"]').dispatchEvent(new PointerEvent("pointerenter"))`);
  const free = await run(`document.getElementById("readout-does").textContent`);
  check(free === "No function yet.", "P01 pointed at: the help says it is free", free);
  check(errors.length === 0, "no errors", errors.join(" | "));
  ws.close();
} catch (err) {
  check(false, "the page runs", err.message);
} finally {
  browser.kill();
  server.close();
}

if (failed) {
  console.log(`${failed} failed`);
  process.exit(1);
}
