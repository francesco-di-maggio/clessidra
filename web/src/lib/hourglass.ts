// The hourglass between Position and Length: the loop as sand. Above the neck,
// what is still ahead of the playhead, below it what has played. Once: it
// runs out and stays empty until Trig. Turn and Loop: full again at the end.
// A thread of sand falls while the playhead moves.

import { PARAM } from "./engine";

const WIDTH = 0.62; // of the height
const NECK = 0.07; // half its width, of the bulb's
const SLICES = 40; // down each bulb

export class Hourglass {
  private ctx: CanvasRenderingContext2D;
  private params: Float32Array = new Float32Array(0);

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => this.draw());
    new MutationObserver(() => this.draw()).observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme"] });
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  update(params: Float32Array) {
    // Drawn again only when what it shows changes
    const shown = [PARAM.sandLeft, PARAM.speed, PARAM.freeze, PARAM.stop];
    const same = this.params.length === params.length && shown.every((i) => this.params[i] === params[i]);
    this.params = params.slice();
    if (!same) this.draw();
  }

  draw() {
    const { canvas, ctx, params: p } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H || p.length <= PARAM.sandLeft) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);
    const accent = css.getPropertyValue("--accent");

    const top = 3;
    const bottom = H - 3;
    const mid = H / 2;
    const bulb = (bottom - top) / 2 - 2; // a bulb's height, inside the caps
    const wide = (WIDTH * (bottom - top)) / 2;
    // Half the glass's width at t of a bulb: 0 at the neck, 1 at its cap
    const half = (t: number) => wide * (NECK + (1 - NECK) * (1 - (1 - t) ** 2));
    const y = (t: number, side: number) => mid + side * t * bulb;

    // The sand, a share of a bulb's area: above, resting on the neck, below,
    // on the cap
    const area = (from: number, to: number) => {
      let sum = 0;
      for (let k = 0; k < SLICES; k++) {
        const t = (k + 0.5) / SLICES;
        if (t >= from && t < to) sum += half(t);
      }
      return sum;
    };
    const whole = area(0, 1);
    // Up to where the sand reaches, from the neck or from the cap
    const level = (share: number, fromCap: boolean) => {
      let sum = 0;
      for (let k = 0; k < SLICES; k++) {
        const t = fromCap ? 1 - (k + 0.5) / SLICES : (k + 0.5) / SLICES;
        sum += half(t);
        if (sum >= share * whole) return fromCap ? 1 - (k + 1) / SLICES : (k + 1) / SLICES;
      }
      return fromCap ? 0 : 1;
    };
    const sand = (from: number, to: number, side: number) => {
      if (to <= from) return;
      ctx.beginPath();
      for (let k = 0; k <= SLICES; k++) {
        const t = from + ((to - from) * k) / SLICES;
        ctx.lineTo(W / 2 - half(t) + 1.5, y(t, side));
      }
      for (let k = SLICES; k >= 0; k--) {
        const t = from + ((to - from) * k) / SLICES;
        ctx.lineTo(W / 2 + half(t) - 1.5, y(t, side));
      }
      ctx.closePath();
      ctx.fillStyle = accent;
      ctx.fill();
    };
    const left = Math.min(Math.max(p[PARAM.sandLeft], 0), 1);
    const above = left > 0 ? level(left, false) : 0;
    const below = left < 1 ? level(1 - left, true) : 1;
    sand(0, above, -1);
    sand(below, 1, 1);
    // Falling: the playhead moving, sand still above
    const moving = !p[PARAM.freeze] && !p[PARAM.stop] && p[PARAM.speed] !== 0 && left > 0;
    if (moving) {
      ctx.strokeStyle = accent;
      ctx.lineWidth = 1.5;
      ctx.setLineDash([1.5, 3]);
      ctx.beginPath();
      ctx.moveTo(W / 2, mid);
      ctx.lineTo(W / 2, y(below, 1));
      ctx.stroke();
      ctx.setLineDash([]);
    }

    // The glass: its sides and its caps
    ctx.strokeStyle = css.getPropertyValue("--ink");
    ctx.lineWidth = 1.5;
    ctx.lineCap = "round";
    ctx.beginPath();
    for (const side of [-1, 1]) {
      for (let k = 0; k <= 2 * SLICES; k++) {
        const t = k / SLICES - 1; // from the upper cap to the lower
        const x = W / 2 + side * half(Math.abs(t));
        if (k === 0) ctx.moveTo(x, y(1, -1));
        else ctx.lineTo(x, mid + t * bulb);
      }
    }
    for (const side of [-1, 1]) {
      ctx.moveTo(W / 2 - wide - 3, y(1, side) + side * 2);
      ctx.lineTo(W / 2 + wide + 3, y(1, side) + side * 2);
    }
    ctx.stroke();
  }
}
