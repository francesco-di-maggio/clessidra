// Grains falling slowly down a canvas, in tan as on the faceplate, drifting a
// little as in still air. The same grains from page to page: where they were
// is kept for the tab and they go on falling from there. Still under reduced
// motion: drawn once where they are.
interface Grain { x: number; y: number; r: number; speed: number; drift: number; alpha: number }

const AREA = 18000; // px² a grain
const SPEED = [4, 14]; // px/s down
const DRIFT = 1.5; // px/s sideways, at most, either way
// The faceplate's dust: S to XL (its radii 0.24, 0.3, 0.36, 0.46) and a rare
// XXL, the small ones most
const SIZES = [0.8, 1.0, 1.2, 1.55, 2.1]; // px, radius
const ODDS = [0.38, 0.3, 0.2, 0.08, 0.04];
const KEY = "clessidra-dust";

function radius() {
  let u = Math.random();
  for (let i = 0; i < SIZES.length; i++) if ((u -= ODDS[i]) < 0) return SIZES[i];
  return SIZES[0];
}

export function fall(canvas: HTMLCanvasElement) {
  const ctx = canvas.getContext("2d")!;
  const still = matchMedia("(prefers-reduced-motion: reduce)");
  let grains: Grain[] = [];
  let W = 0;
  let H = 0;
  let last = 0;
  let frame = 0;
  let first = true;

  const grain = (y: number): Grain => ({
    x: Math.random() * W,
    y,
    r: radius(),
    speed: SPEED[0] + Math.random() * (SPEED[1] - SPEED[0]),
    drift: (2 * Math.random() - 1) * DRIFT,
    alpha: 0.15 + Math.random() * 0.35,
  });

  // Each grain on, by the time since the page before
  function move(g: Grain, dt: number) {
    g.y += g.speed * dt;
    g.x = (((g.x + g.drift * dt) % W) + W) % W;
    if (g.y > H + 2) Object.assign(g, grain(-2 - ((g.y - H) % H)));
  }

  // The grains of the page before, on the same window, else new ones
  function kept(n: number) {
    try {
      const saved = JSON.parse(sessionStorage.getItem(KEY) ?? "null");
      if (saved?.W === W && saved?.H === H && Array.isArray(saved.grains)) {
        const dt = still.matches ? 0 : Math.min((Date.now() - saved.at) / 1000, 60);
        const grains: Grain[] = saved.grains;
        for (const g of grains) move(g, dt);
        return grains;
      }
    } catch {}
    return Array.from({ length: n }, () => grain(Math.random() * H));
  }

  function size() {
    const dpr = devicePixelRatio || 1;
    W = canvas.clientWidth;
    H = canvas.clientHeight;
    canvas.width = Math.round(W * dpr);
    canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    const n = Math.round((W * H) / AREA);
    grains = first ? kept(n) : Array.from({ length: n }, () => grain(Math.random() * H));
    first = false;
    draw();
  }

  function draw() {
    const tan = getComputedStyle(canvas).getPropertyValue("--tan");
    ctx.clearRect(0, 0, W, H);
    ctx.fillStyle = tan;
    for (const g of grains) {
      ctx.globalAlpha = g.alpha;
      ctx.beginPath();
      ctx.arc(g.x, g.y, g.r, 0, 2 * Math.PI);
      ctx.fill();
    }
    ctx.globalAlpha = 1;
  }

  function tick(now: number) {
    const dt = last ? Math.min((now - last) / 1000, 0.1) : 0;
    last = now;
    for (const g of grains) move(g, dt);
    draw();
    frame = requestAnimationFrame(tick);
  }

  function run() {
    cancelAnimationFrame(frame);
    last = 0;
    if (!still.matches) frame = requestAnimationFrame(tick);
  }

  addEventListener("pagehide", () => {
    try {
      sessionStorage.setItem(KEY, JSON.stringify({ W, H, at: Date.now(), grains }));
    } catch {}
  });
  new ResizeObserver(size).observe(canvas);
  still.addEventListener("change", run);
  run();
}
