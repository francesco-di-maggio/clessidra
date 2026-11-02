// The sound as the firmware holds it: the 8 s buffer, where the input is
// written, the selection the playhead moves through, the slices, and every
// grain at the point it reads, high for left and low for right.

import { SLICE_PADS } from "./panel.js";

const RATE = 48000;
const V = { write: 0, heldEnd: 1, sound: 2, selStart: 3, selLength: 4, liveStart: 5, liveLength: 6, playhead: 7, livePlayhead: 8, slice: 9, edges: 10 };
const NUM_SLICES = 7;
const GRAIN_FIELDS = 4;
const FILL = 6; // cl_params

// Grain::Render's window at progress x for shape: Hann at 0.5, percussive
// towards 1, reversed towards 0
export function windowAt(x, shape) {
  const skew = 2 * Math.min(Math.max(shape, 0), 1) - 1;
  const percussive = Math.abs(skew);
  if (skew < 0) x = 1 - x;
  const peak = 0.5 - 0.49 * percussive;
  const rise = (t) => Math.sin((Math.PI / 2) * t) ** 2;
  if (x < peak) return rise(x / peak);
  const fall = Math.min(1, (x - peak) / (1 - peak));
  const hann = rise(1 - fall);
  return hann + percussive * (Math.exp(-5 * fall) * Math.cos((Math.PI / 2) * fall) - hann);
}

export class Sand {
  constructor(canvas, samples) {
    this.canvas = canvas;
    this.ctx = canvas.getContext("2d");
    this.samples = samples;
    this.frame = null;
    this.colours();
    matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => this.colours());
    new MutationObserver(() => this.colours()).observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme"] });
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  colours() {
    const css = getComputedStyle(document.documentElement);
    const get = (name) => css.getPropertyValue(name).trim();
    this.c = { ink: get("--ink"), dim: get("--dim"), line: get("--line"), accent: get("--accent"), surface: get("--surface"), mono: get("--mono") };
    this.draw();
  }

  update(frame) {
    this.frame = frame;
    this.draw();
  }

  draw() {
    const { canvas, ctx, c } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);

    const pad = 14;
    const width = W - 2 * pad;
    const x = (s) => pad + (((s % this.samples) + this.samples) % this.samples) / this.samples * width;
    const top = 12;
    const lane = { top: 22, bottom: 92 }; // grains: left high, right low
    const wave = { mid: 140, half: 34 };
    const bottom = 182;
    const axis = 232;

    // Seconds
    ctx.font = `500 10px ${c.mono}`;
    ctx.textAlign = "center";
    ctx.fillStyle = c.dim;
    ctx.strokeStyle = c.line;
    ctx.lineWidth = 1;
    const seconds = this.samples / RATE;
    for (let s = 0; s <= seconds; s++) {
      const px = pad + (s / seconds) * width;
      line(ctx, px, axis - 8, px, axis - 2);
      ctx.fillText(s === seconds ? `${s} s` : `${s}`, px, axis + 10);
    }
    line(ctx, pad, axis - 8, W - pad, axis - 8);

    const f = this.frame;
    if (!f) {
      ctx.fillStyle = c.dim;
      ctx.font = `500 11px ${c.mono}`;
      ctx.fillText("PRESS START", W / 2, wave.mid + 4);
      return;
    }
    const v = f.view;
    const writing = f.params[FILL] > 0;
    // At Fill 100 % every grain follows the input: nothing reads the held sound
    const holding = v[V.sound] > 0 && f.params[FILL] < 1;
    const sound = v[V.sound];
    // The slices the pads play: at the held sound's onsets, or equal parts of the input
    const edge = (i) => (holding ? v[V.edges + i] : v[V.liveStart] + (i / NUM_SLICES) * v[V.liveLength]);

    // The input's window, behind the write point
    if (writing) {
      ctx.fillStyle = withAlpha(c.accent, 0.1);
      span(this.samples, v[V.liveStart], v[V.liveLength], (a, b) => ctx.fillRect(x(a), top, x(b) - x(a), bottom - top));
    }
    // The held selection
    if (holding) {
      ctx.fillStyle = withAlpha(c.ink, 0.06);
      ctx.strokeStyle = c.ink;
      span(this.samples, v[V.selStart], v[V.selLength], (a, b) => ctx.fillRect(x(a), top, x(b) - x(a), bottom - top));
      ctx.lineWidth = 1;
      line(ctx, x(v[V.selStart]), top, x(v[V.selStart]), bottom);
      line(ctx, x(v[V.selStart] + v[V.selLength] - 1), top, x(v[V.selStart] + v[V.selLength] - 1), bottom);
    }

    // Waveform: each column's peak
    const peaks = f.peaks;
    const cw = width / peaks.length;
    ctx.fillStyle = c.dim;
    for (let i = 0; i < peaks.length; i++) {
      const h = Math.min(1, peaks[i]) * wave.half;
      if (h > 0.25) ctx.fillRect(pad + i * cw, wave.mid - h, Math.max(cw, 1), 2 * h);
    }
    ctx.strokeStyle = c.line;
    line(ctx, pad, wave.mid, W - pad, wave.mid);

    // Slices, under the sound
    if (sound > 0) {
      ctx.strokeStyle = c.dim;
      ctx.font = `500 10px ${c.mono}`;
      for (let i = 0; i <= NUM_SLICES; i++) line(ctx, x(edge(i)), bottom + 4, x(edge(i)), bottom + 12);
      for (let i = 0; i < NUM_SLICES; i++) {
        const a = x(edge(i));
        const b = x(edge(i + 1) - 1);
        if (b - a < 24) continue;
        ctx.fillStyle = v[V.slice] === i ? c.ink : c.dim;
        ctx.fillText(`P${String(SLICE_PADS[i]).padStart(2, "0")}`, (a + b) / 2, bottom + 24);
      }
    }

    // Playheads and the write point
    ctx.lineWidth = 1.5;
    ctx.strokeStyle = c.ink;
    ctx.fillStyle = c.ink;
    if (holding) marker(ctx, x(v[V.playhead]), top, bottom);
    if (writing) {
      ctx.strokeStyle = c.accent;
      ctx.fillStyle = c.accent;
      ctx.setLineDash([3, 3]);
      line(ctx, x(v[V.livePlayhead]), top, x(v[V.livePlayhead]), bottom);
      ctx.setLineDash([]);
    }
    ctx.strokeStyle = writing ? c.accent : c.dim;
    ctx.lineWidth = writing ? 2 : 1;
    line(ctx, x(v[V.write]), 4, x(v[V.write]), bottom);

    // Grains
    const g = f.grains;
    for (let i = 0; i < g.length; i += GRAIN_FIELDS) {
      const level = windowAt(g[i + 1], g[i + 2]);
      const gx = pad + g[i] * width;
      const gy = lane.top + g[i + 3] * (lane.bottom - lane.top);
      ctx.fillStyle = withAlpha(c.ink, 0.2 + 0.8 * level);
      ctx.beginPath();
      ctx.arc(gx, gy, 1.5 + 4.5 * level, 0, 2 * Math.PI);
      ctx.fill();
    }
    ctx.fillStyle = c.dim;
    ctx.textAlign = "left";
    ctx.fillText("L", 2, lane.top + 4);
    ctx.fillText("R", 2, lane.bottom + 4);
  }
}

// Calls draw(a, b) for each part of a span that wraps round the buffer
function span(samples, start, length, draw) {
  let a = ((start % samples) + samples) % samples;
  const end = a + Math.min(length, samples);
  if (end <= samples) return draw(a, end - 0.5);
  draw(a, samples - 0.5);
  draw(0, end - samples);
}

function line(ctx, x0, y0, x1, y1) {
  ctx.beginPath();
  ctx.moveTo(x0, y0);
  ctx.lineTo(x1, y1);
  ctx.stroke();
}

function marker(ctx, x, top, bottom) {
  line(ctx, x, top, x, bottom);
  ctx.beginPath();
  ctx.moveTo(x - 4, top - 6);
  ctx.lineTo(x + 4, top - 6);
  ctx.lineTo(x, top);
  ctx.fill();
}

function withAlpha(colour, alpha) {
  const hex = colour.replace("#", "");
  const n = parseInt(hex.length === 3 ? [...hex].map((h) => h + h).join("") : hex, 16);
  return `rgb(${(n >> 16) & 255} ${(n >> 8) & 255} ${n & 255} / ${alpha})`;
}
