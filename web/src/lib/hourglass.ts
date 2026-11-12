// The hourglass between Position and Length: the loop as sand. One bulb holds
// the loop still ahead of the playhead, the other what has played. It stands
// as the playhead moves: upright at full speed, leaning as it slows, on its
// side when frozen or stopped, turned over in reverse and when Turn turns it.
// The sand lies level in each bulb, in dots on a grid as the recording is
// drawn, and falls in a thread while it runs, faster with Speed. Nothing
// recorded: the glass alone, standing the same way.

import { PARAM } from "./engine";

type Point = [number, number];

// A knob beside it, px, as measured on the page: its dial's arc across, and
// the top of the name's letters below its line, as the foot of the value's
// figures above theirs, less half a stroke. The knob is level about its dial.
const ARC = 38;
const EDGE = 4.75;
const CAP = 3; // px, a cap past the glass at each side
const NECK = 0.07; // half its width, of the bulb's
const STEPS = 24; // along each side of a bulb
const INSET = 2; // px, the sand from the glass
const DOT = 3; // the grid, px, as the input's
const SIZE = 1.5; // a dot, px
const EASE = 0.25; // of the way to its angle, per frame
const SETTLED = 0.005; // rad
const FALL = 12; // px a second, the thread at 1×: at 4× still under half a grid step a frame
const FRESH_MS = 200; // without news for this long: the thread still
const DROP_MS = 700; // one grain from the neck to the sand, falling faster
const TALL = 0.85; // of the room upright, about its middle

export class Hourglass {
  private ctx: CanvasRenderingContext2D;
  private params: Float32Array = new Float32Array(0);
  private sound = false; // something recorded
  private angle = 0; // rad, clockwise from upright
  private frame = 0;
  private stamp = 0; // when the parameters last came, ms
  private fallen = 0; // px the thread has moved, within a step of the grid
  private last = 0; // the frame before, ms
  private dropped = -Infinity; // when the sand last fell a step, ms

  // single: one grain falls each time the sand falls a step, as a clock's
  // seconds, instead of a thread
  constructor(private canvas: HTMLCanvasElement, private single = false) {
    this.ctx = canvas.getContext("2d")!;
    matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => this.draw());
    new MutationObserver(() => this.draw()).observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme"] });
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  update(params: Float32Array, sound: boolean) {
    // Drawn again only when what it shows changes
    const shown = [PARAM.sandLeft, PARAM.speed, PARAM.freeze, PARAM.stop, PARAM.heading];
    const same = this.sound === sound && this.params.length === params.length && shown.every((i) => this.params[i] === params[i]);
    if (this.single && this.params[PARAM.sandLeft] !== params[PARAM.sandLeft]) this.dropped = performance.now();
    this.params = params.slice();
    this.sound = sound;
    this.stamp = performance.now();
    if ((!same || this.flowing()) && !this.frame) this.frame = requestAnimationFrame(this.tick);
  }

  // Which way the playhead moves through the loop, how fast, and how fast up
  // to 1: the glass is no more than upright
  private motion() {
    const p = this.params;
    const still = p[PARAM.freeze] || p[PARAM.stop];
    const way = (p[PARAM.speed] < 0 ? -1 : 1) * (p[PARAM.heading] < 0 ? -1 : 1);
    const speed = still ? 0 : Math.abs(p[PARAM.speed]);
    return { way, speed, pace: Math.min(speed, 1) };
  }

  // Sand falling: a recording, the playhead moving, some of the loop ahead
  private flowing() {
    return this.sound && this.motion().speed > 0 && this.params[PARAM.sandLeft] > 0;
  }

  // A frame: the glass turns towards where it should stand, the thread falls
  private tick = (now: number) => {
    const { way, speed, pace } = this.motion();
    const to = (Math.PI / 2) * (1 - way * pace);
    this.angle += (to - this.angle) * EASE;
    const settled = Math.abs(to - this.angle) < SETTLED;
    if (settled) this.angle = to;
    const running = this.single ? now - this.dropped < DROP_MS : this.flowing() && now - this.stamp < FRESH_MS;
    if (running) this.fallen = (this.fallen + (FALL * speed * Math.min(now - this.last, 50)) / 1000) % DOT;
    this.last = now;
    this.draw();
    this.frame = settled && !running ? 0 : requestAnimationFrame(this.tick);
  };

  draw() {
    const { canvas, ctx, params: p } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H || p.length <= PARAM.heading) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);
    const accent = css.getPropertyValue("--accent");

    // Upright, most of the room from the top of a knob's name to the foot of
    // its value, about its middle, as wide as a dial's arc. It turns about its
    // neck, level with the dials: on its side, as tall as their arcs.
    const tall = (H - 2 * EDGE) * TALL;
    const bulb = tall / 2 - 2;
    const wide = ARC / 2 - CAP;
    const cx = W / 2;
    const cy = H / 2;
    // Half its width at t of a bulb, 0 at the neck, 1 at its cap
    const half = (t: number, inset: number) => Math.max(wide * (NECK + (1 - NECK) * (1 - (1 - t) ** 2)) - inset, 0);
    const cos = Math.cos(this.angle);
    const sin = Math.sin(this.angle);
    const turned = ([x, y]: Point): Point => [cx + x * cos - y * sin, cy + x * sin + y * cos];
    // The inside of a bulb, -1 the one above when upright, as the page sees it
    const inside = (side: number) => {
      const points: Point[] = [];
      for (let k = 0; k <= STEPS; k++) points.push([-half(k / STEPS, INSET), (side * (k * (bulb - INSET))) / STEPS]);
      for (let k = STEPS; k >= 0; k--) points.push([half(k / STEPS, INSET), (side * (k * (bulb - INSET))) / STEPS]);
      return points.map(turned);
    };
    const area = (points: Point[]) => {
      let sum = 0;
      for (let i = 0; i < points.length; i++) {
        const [x0, y0] = points[i];
        const [x1, y1] = points[(i + 1) % points.length];
        sum += x0 * y1 - x1 * y0;
      }
      return Math.abs(sum) / 2;
    };
    // What of a shape lies below a level
    const below = (points: Point[], level: number) => {
      const out: Point[] = [];
      for (let i = 0; i < points.length; i++) {
        const a = points[i];
        const b = points[(i + 1) % points.length];
        if (a[1] >= level) out.push(a);
        if (a[1] >= level !== b[1] >= level) out.push([a[0] + ((b[0] - a[0]) * (level - a[1])) / (b[1] - a[1]), level]);
      }
      return out;
    };
    // A share of a bulb as sand lying level in it: the sand, and its surface
    const settle = (points: Point[], share: number) => {
      const ys = points.map((q) => q[1]);
      let high = Math.min(...ys);
      let low = Math.max(...ys);
      const whole = area(points);
      for (let i = 0; i < 20; i++) {
        const level = (high + low) / 2;
        if (area(below(points, level)) > share * whole) high = level;
        else low = level;
      }
      return { sand: below(points, low), level: low };
    };
    const within = (points: Point[], x: number, y: number) => {
      let inside = false;
      for (let i = 0, j = points.length - 1; i < points.length; j = i++) {
        const [xi, yi] = points[i];
        const [xj, yj] = points[j];
        if (yi > y !== yj > y && x < xi + ((xj - xi) * (y - yi)) / (yj - yi)) inside = !inside;
      }
      return inside;
    };
    // A shape as dots: every point of the grid inside it, the grid about the
    // neck
    const reach = Math.ceil(bulb / DOT) + 1;
    const dots = (points: Point[]) => {
      if (points.length < 3) return;
      for (let i = -reach; i <= reach; i++) {
        for (let j = -reach; j <= reach; j++) {
          const x = cx + i * DOT;
          const y = cy + j * DOT;
          if (within(points, x, y)) ctx.rect(x - SIZE / 2, y - SIZE / 2, SIZE, SIZE);
        }
      }
    };

    if (this.sound) {
      // The loop ahead in the bulb above while it runs forward, the other in
      // reverse: the one that empties is the one on top
      const { way, speed } = this.motion();
      const left = Math.min(Math.max(p[PARAM.sandLeft], 0), 1);
      const ahead = way > 0 ? -1 : 1;
      const from = settle(inside(ahead), left);
      const into = settle(inside(-ahead), 1 - left);
      ctx.fillStyle = accent;
      ctx.beginPath();
      dots(from.sand);
      dots(into.sand);
      // The thread, from the neck down to the sand below, its dots moving
      // down
      const lower = inside(-ahead);
      if (this.single) {
        // One grain, from the neck down to the sand, falling faster
        const t = (performance.now() - this.dropped) / DROP_MS;
        if (t < 1) ctx.rect(cx - SIZE / 2, cy + (into.level - cy) * t * t - SIZE / 2, SIZE, SIZE);
      } else {
        for (let y = cy + this.fallen; speed > 0 && left > 0 && y < into.level; y += DOT) {
          if (y - cy < 2 * DOT || within(lower, cx, y)) ctx.rect(cx - SIZE / 2, y - SIZE / 2, SIZE, SIZE);
        }
      }
      ctx.fill();
    }

    // The glass: its sides and its caps
    ctx.save();
    ctx.translate(cx, cy);
    ctx.rotate(this.angle);
    ctx.strokeStyle = css.getPropertyValue("--ink");
    ctx.lineWidth = 1.5;
    ctx.lineCap = "round";
    ctx.beginPath();
    for (const side of [-1, 1]) {
      for (let k = -STEPS; k <= STEPS; k++) {
        const x = side * half(Math.abs(k) / STEPS, 0);
        if (k === -STEPS) ctx.moveTo(x, -bulb);
        else ctx.lineTo(x, (k * bulb) / STEPS);
      }
      ctx.moveTo(-wide - CAP, side * (bulb + 2));
      ctx.lineTo(wide + CAP, side * (bulb + 2));
    }
    ctx.stroke();
    ctx.restore();
  }
}
