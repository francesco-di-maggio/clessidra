// The sand: the recording playing, the selection the playhead moves through,
// the slices, and every grain at the point it reads, high for left and low for
// right. Laid out to the canvas's size.

import { GRAIN_FIELDS, NUM_SLICES, RATE, VIEW, type Frame } from "./engine";

// Grain::Render's window at progress x for shape: Hann at 0.5, percussive
// towards 1, reversed towards 0
export function windowAt(x: number, shape: number) {
  const skew = 2 * Math.min(Math.max(shape, 0), 1) - 1;
  const percussive = Math.abs(skew);
  if (skew < 0) x = 1 - x;
  const peak = 0.5 - 0.49 * percussive;
  const rise = (t: number) => Math.sin((Math.PI / 2) * t) ** 2;
  if (x < peak) return rise(x / peak);
  const fall = Math.min(1, (x - peak) / (1 - peak));
  const hann = rise(1 - fall);
  return hann + percussive * (Math.exp(-5 * fall) * Math.cos((Math.PI / 2) * fall) - hann);
}

interface Colours {
  surface: string;
  ink: string;
  dim: string;
  line: string;
  accent: string;
  mono: string;
}

const DOT = 4; // the waveform's grid, px

export class Sand {
  private ctx: CanvasRenderingContext2D;
  private frame: Frame | null = null;
  private c!: Colours;
  private dots = document.createElement("canvas"); // the waveform, drawn when it changes
  private dotsKey = "";
  recording = 0; // seconds held, 0 when not recording
  started = false;

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    this.colours();
    matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => this.colours());
    new MutationObserver(() => this.colours()).observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme"] });
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  private colours() {
    const css = getComputedStyle(document.documentElement);
    const get = (name: string) => css.getPropertyValue(name).trim();
    this.c = { surface: get("--surface"), ink: get("--ink"), dim: get("--dim"), line: get("--line"), accent: get("--accent"), mono: get("--mono") };
    this.dotsKey = "";
    this.draw();
  }

  update(frame: Frame) {
    this.frame = frame;
    this.draw();
  }

  draw() {
    const { canvas, ctx, c } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);

    const pad = 14;
    const width = W - 2 * pad;
    const top = 18;
    const axis = H - 14; // seconds
    const bottom = axis - 38; // the slices below it
    const mid = (top + bottom) / 2;
    const half = (bottom - top) / 2 - 6;
    const f = this.frame;
    const sound = f ? f.view[VIEW.sound] : 0;
    const x = (s: number) => pad + (s / sound) * width;

    ctx.font = `500 11px ${c.mono}`;
    ctx.textBaseline = "middle";
    if (this.recording) {
      ctx.textAlign = "right";
      ctx.fillStyle = c.accent;
      ctx.fillText(`● REC ${this.recording.toFixed(1)} S`, W - pad, 9);
    }
    ctx.textAlign = "center";
    if (!f || !sound) {
      ctx.fillStyle = this.recording ? c.accent : c.dim;
      ctx.fillText(this.recording ? "RECORDING" : this.started ? "LISTENING · TAP OR HOLD REC" : "PRESS START", W / 2, mid);
      return;
    }
    const v = f.view;

    // Seconds of the recording
    ctx.font = `500 10px ${c.mono}`;
    ctx.fillStyle = c.dim;
    ctx.strokeStyle = c.line;
    ctx.lineWidth = 1;
    const seconds = sound / RATE;
    const step = seconds > 16 ? 5 : seconds > 6 ? 2 : seconds > 2 ? 1 : 0.5;
    line(ctx, pad, axis - 8, W - pad, axis - 8);
    for (let s = 0; s <= seconds + 1e-6; s += step) {
      const px = pad + (s / seconds) * width;
      line(ctx, px, axis - 8, px, axis - 3);
      if (px < W - pad - 36) ctx.fillText(`${s}`, px, axis + 3);
    }
    ctx.textAlign = "right";
    ctx.fillText(`${seconds.toFixed(1)} s`, W - pad, axis + 3);
    ctx.textAlign = "center";

    // The waveform as dots, ink inside the selection; brackets at its ends
    const a = x(v[VIEW.selStart]);
    const b = x(v[VIEW.selStart] + v[VIEW.selLength]);
    this.drawDots(f.peaks, W, H, pad, width, mid, half, a, b, dpr);
    ctx.drawImage(this.dots, 0, 0, W, H);
    ctx.strokeStyle = c.ink;
    ctx.lineWidth = 1.5;
    bracket(ctx, a, top, bottom, 1);
    bracket(ctx, b, top, bottom, -1);

    // Slices, under the sound
    ctx.strokeStyle = c.dim;
    ctx.font = `500 10px ${c.mono}`;
    for (let i = 0; i <= NUM_SLICES; i++) line(ctx, x(v[VIEW.edges + i]), bottom + 6, x(v[VIEW.edges + i]), bottom + 14);
    for (let i = 0; i < NUM_SLICES; i++) {
      const sa = x(v[VIEW.edges + i]);
      const sb = x(v[VIEW.edges + i + 1]);
      if (sb - sa < 14) continue;
      ctx.fillStyle = v[VIEW.slice] === i ? c.accent : c.dim;
      ctx.fillText(`${i + 1}`, (sa + sb) / 2, bottom + 22);
    }

    // The playhead: now
    ctx.lineWidth = 1.5;
    ctx.strokeStyle = c.accent;
    ctx.fillStyle = c.accent;
    const px = x(v[VIEW.playhead]);
    line(ctx, px, top, px, bottom);
    ctx.beginPath();
    ctx.moveTo(px - 4, top - 6);
    ctx.lineTo(px + 4, top - 6);
    ctx.lineTo(px, top);
    ctx.fill();

    // Grains over the waveform, by pan, each cut clear of the dots
    const g = f.grains;
    for (let i = 0; i < g.length; i += GRAIN_FIELDS) {
      const level = windowAt(g[i + 1], g[i + 2]);
      const gx = pad + g[i] * width;
      const gy = top + 8 + g[i + 3] * (bottom - top - 16);
      const r = 2 + 4 * level;
      ctx.fillStyle = c.surface;
      ctx.beginPath();
      ctx.arc(gx, gy, r + 2.5, 0, 2 * Math.PI);
      ctx.fill();
      ctx.fillStyle = withAlpha(c.ink, 0.35 + 0.65 * level);
      ctx.beginPath();
      ctx.arc(gx, gy, r, 0, 2 * Math.PI);
      ctx.fill();
    }
  }

  // Columns of dots on a DOT grid, each up to its peak, mirrored about mid
  private drawDots(peaks: Float32Array, W: number, H: number, pad: number, width: number, mid: number, half: number, a: number, b: number, dpr: number) {
    let sum = 0;
    for (let i = 0; i < peaks.length; i += 7) sum += peaks[i];
    const key = `${W}x${H}:${dpr}:${a.toFixed(1)}:${b.toFixed(1)}:${sum.toFixed(4)}`;
    if (key === this.dotsKey) return;
    this.dotsKey = key;
    const d = this.dots;
    d.width = Math.round(W * dpr);
    d.height = Math.round(H * dpr);
    const g = d.getContext("2d")!;
    g.setTransform(dpr, 0, 0, dpr, 0, 0);
    const inside = new Path2D();
    const outside = new Path2D();
    const cols = Math.floor(width / DOT);
    const rows = Math.floor(half / DOT);
    for (let k = 0; k < cols; k++) {
      const i0 = Math.floor((k / cols) * peaks.length);
      const i1 = Math.max(i0 + 1, Math.floor(((k + 1) / cols) * peaks.length));
      let peak = 0;
      for (let i = i0; i < i1; i++) peak = Math.max(peak, peaks[i]);
      const n = Math.round(Math.min(1, peak) * rows);
      const cx = pad + k * DOT + DOT / 2;
      const path = cx >= a && cx <= b ? inside : outside;
      for (let j = -n; j <= n; j++) path.rect(cx - 1, mid + j * DOT - 1, 2, 2);
    }
    g.fillStyle = this.c.line;
    g.fill(outside);
    g.fillStyle = this.c.ink;
    g.fill(inside);
  }
}

// [ at the start of a span, ] at its end
function bracket(ctx: CanvasRenderingContext2D, x: number, top: number, bottom: number, side: 1 | -1) {
  ctx.beginPath();
  ctx.moveTo(x + 6 * side, top);
  ctx.lineTo(x, top);
  ctx.lineTo(x, bottom);
  ctx.lineTo(x + 6 * side, bottom);
  ctx.stroke();
}

function line(ctx: CanvasRenderingContext2D, x0: number, y0: number, x1: number, y1: number) {
  ctx.beginPath();
  ctx.moveTo(x0, y0);
  ctx.lineTo(x1, y1);
  ctx.stroke();
}

function withAlpha(colour: string, alpha: number) {
  const hex = colour.replace("#", "");
  const n = parseInt(hex.length === 3 ? [...hex].map((h) => h + h).join("") : hex, 16);
  return `rgb(${(n >> 16) & 255} ${(n >> 8) & 255} ${n & 255} / ${alpha})`;
}
