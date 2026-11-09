// The grain tile: one grain's envelope as Shape and Size set it, its length
// on a log axis from the shortest grain to the longest. Faint: the lengths
// Random size reaches.

import { PARAM } from "./engine";
import { windowAt } from "./waveform";

export class GrainShape {
  private ctx: CanvasRenderingContext2D;
  private params: Float32Array = new Float32Array(0);

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  update(params: Float32Array) {
    this.params = params;
    this.draw();
  }

  draw() {
    const { canvas, ctx, params: p } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H || p.length <= PARAM.maxGrainMs) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);

    // Size on a log axis, as the knob sets it
    const min = p[PARAM.minGrainMs];
    const max = Math.max(p[PARAM.maxGrainMs], min * 1.01);
    const at = (ms: number) => Math.min(Math.max(Math.log(ms / min) / Math.log(max / min), 0), 1);
    const used = p[PARAM.grainMs];
    const shape = p[PARAM.shape];
    const octaves = Math.log2(max / min);
    const reach = Math.min(Math.max(Math.sqrt(4 * p[PARAM.randSize]), 0), 1) * 0.25 * octaves;
    const envelope = (ms: number) => {
      const right = 2 + at(ms) * (W - 4);
      ctx.beginPath();
      ctx.moveTo(2, H - 1);
      for (let x = 2; x <= right; x++) ctx.lineTo(x, H - 1 - windowAt((x - 2) / Math.max(right - 2, 1), shape) * (H - 3));
      ctx.lineTo(right, H - 1);
    };
    ctx.lineWidth = 1;
    if (reach > 0) {
      ctx.strokeStyle = css.getPropertyValue("--line");
      for (const ms of [used / 2 ** reach, used * 2 ** reach]) {
        envelope(Math.min(Math.max(ms, min), max));
        ctx.stroke();
      }
    }
    envelope(used);
    ctx.fillStyle = css.getPropertyValue("--accent");
    ctx.globalAlpha = 0.14;
    ctx.fill();
    ctx.globalAlpha = 1;
    ctx.strokeStyle = css.getPropertyValue("--ink");
    ctx.lineWidth = 1.5;
    ctx.stroke();
  }
}
