// The incoming strip: the input over the last seconds, newest at the right, as
// dots, its scale below. What a tap on Rec keeps; the part being recorded in
// orange.

import { INCOMING_SECONDS } from "./engine";

const DOT = 4; // the grid, px

export class Incoming {
  private ctx: CanvasRenderingContext2D;
  private peaks: Float32Array = new Float32Array(0);
  recording = 0; // seconds held, 0 when not recording

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
    const pad = 4;
    const axis = H - 12; // its scale below
    const cols = Math.floor((W - 2 * pad) / DOT);
    const rows = Math.floor((axis - 12) / 2 / DOT);
    const mid = (axis + 4) / 2;
    const held = Math.min(this.recording / INCOMING_SECONDS, 1); // share of the strip, from the right
    const quiet = new Path2D();
    const loud = new Path2D();
    const rec = new Path2D();
    for (let k = 0; k < cols; k++) {
      const i0 = Math.floor((k / cols) * this.peaks.length);
      const i1 = Math.max(i0 + 1, Math.floor(((k + 1) / cols) * this.peaks.length));
      let peak = 0;
      for (let i = i0; i < i1 && i < this.peaks.length; i++) peak = Math.max(peak, this.peaks[i]);
      const n = Math.round(Math.min(1, peak) * rows);
      const cx = pad + k * DOT + DOT / 2;
      const path = k >= cols * (1 - held) ? rec : n > 0 ? loud : quiet;
      for (let j = -n; j <= n; j++) path.rect(cx - 1, mid + j * DOT - 1, 2, 2);
    }
    ctx.fillStyle = css.getPropertyValue("--line");
    ctx.fill(quiet);
    ctx.fillStyle = css.getPropertyValue("--dim");
    ctx.fill(loud);
    ctx.fillStyle = css.getPropertyValue("--accent");
    ctx.fill(rec);
    ctx.font = `500 10px ${css.getPropertyValue("--mono")}`;
    ctx.textBaseline = "middle";
    ctx.fillStyle = css.getPropertyValue("--dim");
    ctx.textAlign = "left";
    ctx.fillText(`−${INCOMING_SECONDS} S`, pad, axis);
    ctx.textAlign = "right";
    ctx.fillText("NOW", W - pad, axis);
  }
}
