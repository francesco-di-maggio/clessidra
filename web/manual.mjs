// Prints the manual page of web/dist with headless Chrome, lays it over
// Synthux's Touch II blank manual (templates/touch-ii-blank.pdf) and writes
// web/public/clessidra-manual.pdf. From the repo: make manual, after make web.
// Chrome from $CHROME, or where it usually is.
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import { existsSync, mkdtempSync, readFileSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { PDFDocument, rgb } from "pdf-lib";
import { serve } from "./serve.mjs";

const PORT = 4330;
const FRAME = 23; // the template's frame, inside its line, in points
const out = new URL("public/clessidra-manual.pdf", import.meta.url).pathname;
const template = new URL("templates/touch-ii-blank.pdf", import.meta.url).pathname;
const chrome = [process.env.CHROME, "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome", "/usr/bin/google-chrome", "/usr/bin/chromium"].find((p) => p && existsSync(p));
if (!chrome) throw new Error("no Chrome: set CHROME");

const server = serve(PORT);

try {
  // Its own profile: a running Chrome would otherwise take the job over
  const profile = mkdtempSync(join(tmpdir(), "clessidra-manual-"));
  // Async: the server above answers from this same process
  const text = join(profile, "text.pdf");
  await promisify(execFile)(chrome, ["--headless=new", `--user-data-dir=${profile}`, "--no-pdf-header-footer", "--virtual-time-budget=5000", `--print-to-pdf=${text}`, `http://localhost:${PORT}/clessidra/manual/`]);
  // The template's page as it is, the text over it, page for page in points.
  // Page 2: the template's frame alone, its inside covered.
  const manual = await PDFDocument.load(readFileSync(template));
  const [blank] = await manual.copyPages(manual, [0]);
  manual.addPage(blank);
  const { width, height } = blank.getSize();
  blank.drawRectangle({ x: FRAME, y: FRAME, width: width - 2 * FRAME, height: height - 2 * FRAME, color: rgb(1, 1, 1) });
  const pages = await manual.embedPdf(readFileSync(text), [0, 1]);
  pages.forEach((page, i) => manual.getPage(i).drawPage(page, { x: 0, y: 0 }));
  manual.setTitle("Clessidra · Manual");
  writeFileSync(out, await manual.save());
  console.log(out);
  // The engine page as it prints
  const engine = new URL("public/clessidra-engine.pdf", import.meta.url).pathname;
  await promisify(execFile)(chrome, ["--headless=new", `--user-data-dir=${profile}`, "--no-pdf-header-footer", "--virtual-time-budget=5000", `--print-to-pdf=${engine}`, `http://localhost:${PORT}/clessidra/engine/`]);
  console.log(engine);
} finally {
  server.close();
}
