// Renders the site's images with headless Chrome: favicon.svg to
// apple-touch-icon.png (180 px) and favicon.png (32 px), images/og.html to
// og.png (1200 × 630). From the repo: make images. Chrome from $CHROME, or
// where it usually is.
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import { existsSync, mkdtempSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

const here = (path) => new URL(path, import.meta.url).pathname;
const chrome = [process.env.CHROME, "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome", "/usr/bin/google-chrome", "/usr/bin/chromium"].find((p) => p && existsSync(p));
if (!chrome) throw new Error("no Chrome: set CHROME");

// Its own profile: a running Chrome would otherwise take the job over
const profile = mkdtempSync(join(tmpdir(), "clessidra-images-"));
const shoot = (page, width, height, out) =>
  promisify(execFile)(chrome, ["--headless=new", `--user-data-dir=${profile}`, "--hide-scrollbars", "--default-background-color=00000000", `--window-size=${width},${height}`, `--screenshot=${out}`, `file://${page}`]);

for (const [size, name] of [[180, "apple-touch-icon.png"], [32, "favicon.png"]]) {
  const page = join(profile, `icon-${size}.html`);
  writeFileSync(page, `<!doctype html><body style="margin:0"><img src="file://${here("public/favicon.svg")}" width="${size}" height="${size}" style="display:block">`);
  await shoot(page, size, size, here(`public/${name}`));
}
await shoot(here("images/og.html"), 1200, 630, here("public/og.png"));
