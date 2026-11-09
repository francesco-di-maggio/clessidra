// The output tile: the sound leaving the firmware as a trace, before the
// browser's Out level, from a rising zero crossing so it stands still.

export class Scope {
  private ctx: CanvasRenderingContext2D;
  private trace: Float32Array = new Float32Array(0);

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  update(trace: Float32Array) {
    this.trace = trace;
    this.draw();
  }

  draw() {
    const { canvas, ctx, trace } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);
    const mid = H / 2;
    ctx.strokeStyle = css.getPropertyValue("--line");
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(0, mid);
    ctx.lineTo(W, mid);
    ctx.stroke();
    if (!trace.length) return;
    ctx.strokeStyle = css.getPropertyValue("--ink");
    ctx.lineWidth = 1.25;
    ctx.beginPath();
    trace.forEach((s, i) => {
      const x = (i / (trace.length - 1)) * W;
      const y = mid - Math.max(-1, Math.min(1, s)) * (mid - 2);
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.stroke();
  }
}
