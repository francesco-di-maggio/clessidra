// The grain tile: the grains sounding now, each its envelope on a short time
// axis, the now line at the centre: what has sounded to its left, what is to
// come to its right. Overlap shows as Density sets it. None sounding: the
// envelope Size and Shape set, faint.

import { GRAIN_FIELDS, PARAM } from "./engine";
import { windowAt } from "./waveform";

export class GrainShape {
  private ctx: CanvasRenderingContext2D;
  private params: Float32Array = new Float32Array(0);
  private grains: Float32Array = new Float32Array(0);

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  update(params: Float32Array, grains: Float32Array = new Float32Array(0)) {
    this.params = params;
    this.grains = grains;
    this.draw();
  }

  draw() {
    const { canvas, ctx, params: p, grains: g } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H || p.length <= PARAM.grainMs) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);
    const ink = css.getPropertyValue("--ink");
    const accent = css.getPropertyValue("--accent");

    // Two grain lengths across, ms
    const span = 2 * p[PARAM.grainMs];
    const x = (ms: number) => W / 2 + (ms / span) * W;
    const floor = H - 1;
    const envelope = (start: number, length: number, shape: number, alpha: number) => {
      ctx.beginPath();
      ctx.moveTo(x(start), floor);
      for (let k = 0; k <= 32; k++) ctx.lineTo(x(start + (k / 32) * length), floor - windowAt(k / 32, shape) * (H - 3));
      ctx.lineTo(x(start + length), floor);
      ctx.globalAlpha = 0.1;
      ctx.fillStyle = accent;
      ctx.fill();
      ctx.globalAlpha = alpha;
      ctx.strokeStyle = ink;
      ctx.lineWidth = 1;
      ctx.stroke();
    };

    const count = Math.floor(g.length / GRAIN_FIELDS);
    if (count === 0) envelope(-p[PARAM.grainMs] / 2, p[PARAM.grainMs], p[PARAM.shape], 0.35);
    for (let i = 0; i < count; i++) {
      const f = i * GRAIN_FIELDS;
      const length = g[f + 5];
      envelope(-g[f + 1] * length, length, g[f + 2], 0.8);
    }

    ctx.globalAlpha = 1;
    ctx.strokeStyle = css.getPropertyValue("--line");
    ctx.beginPath();
    ctx.moveTo(Math.round(W / 2) + 0.5, 0);
    ctx.lineTo(Math.round(W / 2) + 0.5, H);
    ctx.stroke();
  }
}
