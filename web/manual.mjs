// Prints the manual page of web/dist to web/public/clessidra-manual.pdf with
// headless Chrome. From the repo: make manual, after make web. Chrome from
// $CHROME, or where it usually is.
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import { existsSync, mkdtempSync, readFileSync } from "node:fs";
import { createServer } from "node:http";
import { tmpdir } from "node:os";
import { extname, join } from "node:path";

const PORT = 4330;
const dist = new URL("dist/", import.meta.url).pathname;
const out = new URL("public/clessidra-manual.pdf", import.meta.url).pathname;
const chrome = [process.env.CHROME, "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome", "/usr/bin/google-chrome", "/usr/bin/chromium"].find((p) => p && existsSync(p));
if (!chrome) throw new Error("no Chrome: set CHROME");

// web/dist at /clessidra/, as GitHub Pages serves it
const TYPES = { ".html": "text/html", ".js": "text/javascript", ".css": "text/css", ".svg": "image/svg+xml" };
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
}).listen(PORT);

try {
  // Its own profile: a running Chrome would otherwise take the job over
  const profile = mkdtempSync(join(tmpdir(), "clessidra-manual-"));
  // Async: the server above answers from this same process
  await promisify(execFile)(chrome, ["--headless=new", `--user-data-dir=${profile}`, "--no-pdf-header-footer", "--virtual-time-budget=5000", `--print-to-pdf=${out}`, `http://localhost:${PORT}/clessidra/manual/`]);
  console.log(out);
} finally {
  server.close();
}
