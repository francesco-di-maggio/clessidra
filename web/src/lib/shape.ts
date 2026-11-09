// The grain tile: the texture of the grains, drawn relative to their size (the
// size itself is the Recording header's figure). Each grain its envelope, as
// Shape sets it; as many overlapping as Density sounds at once, evenly spaced
// when regular, scattered when random; their widths varied by Random size.
// None at Density's centre: one grain, faint.

import { PARAM } from "./engine";
import { windowAt } from "./waveform";

const MOST = 48; // grains drawn at most

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
    const ink = css.getPropertyValue("--ink");
    const accent = css.getPropertyValue("--accent");

    const size = p[PARAM.grainMs] / 1000; // s
    const density = p[PARAM.density]; // grains per second
    const shape = p[PARAM.shape];
    const octaves = Math.log2(Math.max(p[PARAM.maxGrainMs], p[PARAM.minGrainMs] * 1.01) / p[PARAM.minGrainMs]);
    const reach = Math.min(Math.max(Math.sqrt(4 * p[PARAM.randSize]), 0), 1) * 0.25 * octaves;

    // The same scatter every frame: it changes with the settings, not by itself
    let seed = 7;
    const random = () => ((seed = (seed * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff) * 2 - 1;

    // Long enough for four grains' length, or three starts when sparse
    const span = density > 0 ? Math.max(4 * size, 3 / density) : 2 * size;
    const grains: [number, number][] = []; // start, length, s
    if (density === 0) grains.push([0.5 * size, size]);
    else {
      let t = -size;
      while (t < span && grains.length < MOST) {
        const length = size * 2 ** (random() * reach);
        grains.push([t, length]);
        // Random: nearly exponential intervals of the same mean
        const gap = p[PARAM.randomStarts] ? -Math.log(Math.max(1e-3, 0.5 + 0.5 * random())) : 1;
        t += gap / density;
      }
    }

    const x = (t: number) => (t / span) * W;
    const floor = H - 1;
    for (const [start, length] of grains) {
      ctx.beginPath();
      ctx.moveTo(x(start), floor);
      for (let k = 0; k <= 32; k++) ctx.lineTo(x(start + (k / 32) * length), floor - windowAt(k / 32, shape) * (H - 3));
      ctx.lineTo(x(start + length), floor);
      ctx.globalAlpha = 0.1;
      ctx.fillStyle = accent;
      ctx.fill();
      ctx.globalAlpha = density === 0 ? 0.35 : 0.8;
      ctx.strokeStyle = ink;
      ctx.lineWidth = 1;
      ctx.stroke();
    }
    ctx.globalAlpha = 1;
  }
}
