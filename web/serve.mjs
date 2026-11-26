// web/dist at /clessidra/, as GitHub Pages serves it, for headless Chrome
import { readFileSync } from "node:fs";
import { createServer } from "node:http";
import { extname, join } from "node:path";

const dist = new URL("dist/", import.meta.url).pathname;
const TYPES = { ".html": "text/html", ".js": "text/javascript", ".css": "text/css", ".svg": "image/svg+xml", ".woff2": "font/woff2", ".woff": "font/woff" };

export function serve(port) {
  return createServer((req, res) => {
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
  }).listen(port);
}
