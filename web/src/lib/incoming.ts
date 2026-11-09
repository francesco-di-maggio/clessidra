// The input strip: the input over the seconds a tap on Rec keeps, newest at
// the right, as dots on a linear scale, its scale below. The part being recorded
// in orange.

import { INCOMING_SECONDS, TAP_SECONDS } from "./engine";

const DOT = 3; // the grid, px
const SIZE = 1.5; // a dot, px

export class Incoming {
  private ctx: CanvasRenderingContext2D;
  private peaks: Float32Array = new Float32Array(0);
  recording = 0; // seconds held, 0 when not recording
  seconds = TAP_SECONDS; // shown, the newest of the INCOMING_SECONDS held

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => this.draw());
    new MutationObserver(() => this.draw()).observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme"] });
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  update(peaks: Float32Array) {
    this.peaks = peaks;
    this.draw();
  }

  draw() {
    const { canvas, ctx } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);
    // The tile holds the inset, its scale below: a tick a second
    const pad = 0;
    const axis = H - 12;
    const width = W - 2 * pad;
    const cols = Math.floor(width / DOT);
    const top = 6;
    const bottom = axis - 12;
    const rows = Math.floor((bottom - top) / 2 / DOT);
    const mid = (top + bottom) / 2;
    const held = Math.min(this.recording / this.seconds, 1); // share of the strip, from the right
    const from = this.peaks.length * (1 - this.seconds / INCOMING_SECONDS);
    const span = this.peaks.length - from;
    const quiet = new Path2D();
    const loud = new Path2D();
    const rec = new Path2D();
    for (let k = 0; k < cols; k++) {
      const i0 = Math.floor(from + (k / cols) * span);
      const i1 = Math.max(i0 + 1, Math.floor(from + ((k + 1) / cols) * span));
      let peak = 0;
      for (let i = i0; i < i1 && i < this.peaks.length; i++) peak = Math.max(peak, this.peaks[i]);
      // Linear, as a waveform: a normal level reads thin, the peaks stand out
      const n = Math.round(Math.min(1, peak) * rows);
      const cx = pad + k * DOT + DOT / 2;
      const path = k >= cols * (1 - held) ? rec : n > 0 ? loud : quiet;
      for (let j = -n; j <= n; j++) path.rect(cx - SIZE / 2, mid + j * DOT - SIZE / 2, SIZE, SIZE);
    }
    ctx.fillStyle = css.getPropertyValue("--line");
    ctx.fill(quiet);
    ctx.fillStyle = css.getPropertyValue("--dim");
    ctx.fill(loud);
    ctx.fillStyle = css.getPropertyValue("--accent");
    ctx.fill(rec);
    ctx.strokeStyle = css.getPropertyValue("--line");
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(pad, axis - 7.5);
    ctx.lineTo(W - pad, axis - 7.5);
    for (let s = 0; s <= this.seconds; s++) {
      const x = Math.round(pad + (s / this.seconds) * width) + 0.5;
      ctx.moveTo(x, axis - 7.5);
      ctx.lineTo(x, axis - 3.5);
    }
    ctx.stroke();
    ctx.font = `500 10px ${css.getPropertyValue("--mono")}`;
    ctx.textBaseline = "middle";
    ctx.fillStyle = css.getPropertyValue("--dim");
    ctx.textAlign = "left";
    ctx.fillText(`−${this.seconds} s`, pad, axis + 3);
    ctx.textAlign = "right";
    ctx.fillText("now", W - pad, axis + 3);
  }
}
