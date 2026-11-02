// The Play page in headless Chrome: Start, record with R, and the firmware
// answers. Serves web/dist with astro preview. From the repo: make test, after
// make web. Chrome from $CHROME, or where it usually is.
import { spawn } from "node:child_process";
import { existsSync, mkdtempSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

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

const web = new URL("../web/", import.meta.url).pathname;
// In its own process group: stopping npx alone would leave astro serving
const preview = spawn("npx", ["astro", "preview", "--port", String(PREVIEW)], { cwd: web, stdio: "ignore", detached: true });
const browser = spawn(chrome, ["--headless=new", `--remote-debugging-port=${DEBUG}`, "--autoplay-policy=no-user-gesture-required", `--user-data-dir=${mkdtempSync(join(tmpdir(), "clessidra-"))}`, "about:blank"], { stdio: "ignore" });

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
  const values = await run(`[...document.querySelectorAll(".knob .value")].map((v) => v.textContent)`);
  check(values.length > 0 && values.every((v) => v && v !== "–"), "every knob shows the firmware's value", values.join(" "));
  check(errors.length === 0, "no errors", errors.join(" | "));
  ws.close();
} catch (err) {
  check(false, "the page runs", err.message);
} finally {
  browser.kill();
  process.kill(-preview.pid);
}

if (failed) {
  console.log(`${failed} failed`);
  process.exit(1);
}
