// Grains falling slowly down a canvas, in the accent colour. Still under
// reduced motion: drawn once where they are.
interface Grain { x: number; y: number; r: number; speed: number; alpha: number }

const AREA = 14000; // px² a grain
const SPEED = [4, 14]; // px/s

export function fall(canvas: HTMLCanvasElement) {
  const ctx = canvas.getContext("2d")!;
  const still = matchMedia("(prefers-reduced-motion: reduce)");
  let grains: Grain[] = [];
  let W = 0;
  let H = 0;
  let last = 0;
  let frame = 0;

  const grain = (y: number): Grain => ({
    x: Math.random() * W,
    y,
    r: 0.6 + Math.random() * 0.9,
    speed: SPEED[0] + Math.random() * (SPEED[1] - SPEED[0]),
    alpha: 0.2 + Math.random() * 0.5,
  });

  function size() {
    const dpr = devicePixelRatio || 1;
    W = canvas.clientWidth;
    H = canvas.clientHeight;
    canvas.width = Math.round(W * dpr);
    canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    const n = Math.round((W * H) / AREA);
    grains = Array.from({ length: n }, () => grain(Math.random() * H));
    draw();
  }

  function draw() {
    const accent = getComputedStyle(canvas).getPropertyValue("--accent");
    ctx.clearRect(0, 0, W, H);
    ctx.fillStyle = accent;
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
    for (const g of grains) {
      g.y += g.speed * dt;
      if (g.y > H + 2) Object.assign(g, grain(-2));
    }
    draw();
    frame = requestAnimationFrame(tick);
  }

  function run() {
    cancelAnimationFrame(frame);
    last = 0;
    if (!still.matches) frame = requestAnimationFrame(tick);
  }

  new ResizeObserver(size).observe(canvas);
  still.addEventListener("change", run);
  run();
}
