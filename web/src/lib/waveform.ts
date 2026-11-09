// The recording view: the recording playing, the loop the playhead moves through,
// and every grain at the point it reads, as high as its pitch and as big as its
// size. Laid out to the canvas's size.

import { GRAIN_FIELDS, PARAM, RATE, VIEW, type Frame } from "./engine";

const PITCH_RANGE = 36; // semitones: Pitch's ±24 and Random pitch's ±12
const SIZE_MIN = 5; // ms, CoreUI::kMinGrainMs
const SIZE_MAX = 2000; // CoreUI::kMaxGrainMs

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

export class Waveform {
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
    const top = 18; // the slices' numbers above
    const axis = H - 14; // seconds
    const bottom = axis - 18;
    const mid = (top + bottom) / 2;
    const half = (bottom - top) / 2 - 6;
    const f = this.frame;
    const sound = f ? f.view[VIEW.sound] : 0;
    const x = (s: number) => pad + (s / sound) * width;

    ctx.font = `500 11px ${c.mono}`;
    ctx.textBaseline = "middle";
    ctx.textAlign = "center";
    if (!f || !sound) {
      ctx.fillStyle = this.recording ? c.accent : c.dim;
      ctx.fillText(this.recording ? "RECORDING" : this.started ? "TAP OR HOLD REC" : "PRESS START", W / 2, mid);
      return;
    }
    const v = f.view;
    // Seconds of the recording
    const seconds = sound / RATE;
    this.axis(seconds, W, pad, width, axis);

    // The waveform as dots, ink inside the selection; brackets at its ends.
    // Past the end, it goes on from the start: b beyond the right edge.
    const a = x(v[VIEW.selStart]);
    const b = a + (v[VIEW.selLength] / sound) * width;
    this.drawDots(f.peaks, W, H, pad, width, mid, half, a, b, dpr);
    ctx.drawImage(this.dots, 0, 0, W, H);
    ctx.strokeStyle = c.ink;
    ctx.lineWidth = 1.5;
    bracket(ctx, a, top, bottom, 1);
    bracket(ctx, b > pad + width + 0.5 ? b - width : b, top, bottom, -1);

    // The slices: a hairline where each starts, its number beside it at the
    // top, the first at the bracket
    ctx.strokeStyle = c.line;
    ctx.fillStyle = c.dim;
    ctx.lineWidth = 1;
    ctx.textBaseline = "top";
    ctx.textAlign = "left";
    f.slices.forEach((s, i) => {
      let at = v[VIEW.selStart] + s * v[VIEW.selLength];
      at -= Math.floor(at / sound) * sound;
      const px = x(at);
      if (i > 0) line(ctx, px, top + 2, px, bottom - 2);
      ctx.fillText(`${i + 1}`, px + 4, top + 2);
    });
    ctx.textAlign = "center";

    // Random position: the reach of the grains about the playhead, round the
    // loop, faint
    const reach = 0.5 * Math.min(f.params[PARAM.randPos] ?? 0, 1) * v[VIEW.selLength];
    if (reach > 0) {
      ctx.fillStyle = withAlpha(c.accent, 0.12);
      const from = v[VIEW.playhead] - reach;
      const span = 2 * reach;
      const start = v[VIEW.selStart];
      const length = v[VIEW.selLength];
      // In loop terms, then drawn in up to two parts
      let u = (from - start) / length;
      u -= Math.floor(u);
      const w = span / length;
      const part = (u0: number, u1: number) => ctx.fillRect(x(start + u0 * length), top, x(start + u1 * length) - x(start + u0 * length), bottom - top);
      if (w >= 1) part(0, 1);
      else if (u + w <= 1) part(u, u + w);
      else {
        part(u, 1);
        part(0, u + w - 1);
      }
    }

    // The playhead: now
    ctx.lineWidth = 1.5;
    ctx.strokeStyle = c.accent;
    ctx.fillStyle = c.accent;
    const px = x(v[VIEW.playhead] % sound);
    line(ctx, px, top, px, bottom);
    ctx.beginPath();
    ctx.moveTo(px - 4, top - 6);
    ctx.lineTo(px + 4, top - 6);
    ctx.lineTo(px, top);
    ctx.fill();

    // Grains over the waveform where they read, as high as their pitch (the
    // centre unison, the edges PITCH_RANGE either way), as big as their size,
    // swelling with their window, each cut clear of the dots
    const g = f.grains;
    const lift = (bottom - top) / 2 - 8;
    for (let i = 0; i < g.length; i += GRAIN_FIELDS) {
      const level = windowAt(g[i + 1], g[i + 2]);
      const gx = pad + g[i] * width;
      const gy = (top + bottom) / 2 - (Math.max(-1, Math.min(1, g[i + 4] / PITCH_RANGE)) * lift);
      const size = Math.max(0, Math.min(1, Math.log(g[i + 5] / SIZE_MIN) / Math.log(SIZE_MAX / SIZE_MIN)));
      const r = (2 + 5 * size) * (0.4 + 0.6 * level);
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
  // The seconds of the recording below
  private axis(seconds: number, W: number, pad: number, width: number, axis: number) {
    const { ctx, c } = this;
    ctx.font = `500 10px ${c.mono}`;
    ctx.fillStyle = c.dim;
    ctx.strokeStyle = c.line;
    ctx.lineWidth = 1;
    ctx.textBaseline = "middle";
    ctx.textAlign = "center";
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
  }

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
      const path = (cx >= a && cx <= b) || cx + width <= b ? inside : outside;
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
