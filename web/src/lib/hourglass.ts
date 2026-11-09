// The hourglass tile: the glass drawn from what the firmware does. Sand above
// is what is left of the selection, the pile below what has run. The neck
// opens with Density. Each falling dot is a grain sounding: its height its
// progress, its drift its pan, its brightness its window, its size the grain
// size. Frozen: the glass on its side. Reverse: upside down. All voices
// busy, the neck clogs: grains wait above it.

import { GRAIN_FIELDS, PARAM, VIEW, type Frame } from "./engine";
import { windowAt } from "./waveform";

const DOT = 4; // the sand's grid, px
const MAX_OVERLAP = 16; // CoreUI::kMaxOverlap

interface Colours {
  ink: string;
  dim: string;
  line: string;
  mono: string;
}

export class Hourglass {
  private ctx: CanvasRenderingContext2D;
  private frame: Frame | null = null;
  private c!: Colours;
  private angle = 0; // drawn, degrees
  private raf = 0;
  clogged = false; // grains dropped lately

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
    this.c = { ink: get("--ink"), dim: get("--dim"), line: get("--line"), mono: get("--mono") };
    this.draw();
  }

  update(frame: Frame) {
    this.frame = frame;
    if (!this.raf) this.raf = requestAnimationFrame(() => this.tick());
  }

  // Eases the glass towards the angle Speed sets, and through a flip
  private tick() {
    this.raf = 0;
    const p = this.frame?.params;
    if (!p) return;
    const target = (p[PARAM.speed] < 0 ? 180 : 0) + (p[PARAM.freeze] ? 90 : 0);
    this.angle += (target - this.angle) * 0.18;
    if (Math.abs(target - this.angle) < 0.2) this.angle = target;
    this.draw();
    if (this.angle !== target) this.raf = requestAnimationFrame(() => this.tick());
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

    const f = this.frame;
    const p = f?.params;
    const sound = f ? f.view[VIEW.sound] : 0;

    // The glass, fitted upright or on its side, whichever it is turning to
    // Below the tile's name, inset as the other tiles' drawings
    const side = Math.abs(Math.sin((this.angle * Math.PI) / 180));
    const top = 34;
    const inset = 16;
    const room = H - top - inset;
    const ratio = 0.56;
    const h = (Math.min(room, (W - 2 * inset) / ratio) - 6) * (1 - 0.4 * side);
    const w = h * ratio;
    ctx.save();
    ctx.translate(W / 2, top + room / 2);
    ctx.rotate((this.angle * Math.PI) / 180);
    // Past horizontal, the sand falls the other way: the glass is symmetric,
    // so mirrored it rests on what is now below
    if (Math.cos((this.angle * Math.PI) / 180) < 0) ctx.scale(1, -1);

    const overlap = p ? (p[PARAM.density] * p[PARAM.grainMsSet]) / 1000 : 0;
    const open = overlap > 0 ? Math.log(1 + overlap) / Math.log(1 + MAX_OVERLAP) : 0;
    const neck = Math.max(DOT / 2, open * w * 0.16);
    const glass = outline(w, h, neck);

    // Sand: dots on the grid inside each bulb, up to its level
    const left = sound && p ? Math.min(Math.max(p[PARAM.sandLeft], 0), 1) : 0;
    ctx.fillStyle = c.dim;
    sand(ctx, glass, h, left, true);
    ctx.fillStyle = c.ink;
    sand(ctx, glass, h, sound ? 1 - left : 0, false);

    // The glass over it, in its frame: a plate at each end
    ctx.strokeStyle = c.ink;
    ctx.lineWidth = 1.5;
    ctx.stroke(glass.path);
    ctx.fillStyle = c.ink;
    const plate = w * 0.5;
    ctx.fillRect(-plate, -h / 2 - 3, 2 * plate, 3);
    ctx.fillRect(-plate, h / 2, 2 * plate, 3);

    // Grains falling from the neck to the pile, as they sound
    if (f && p) {
      const pile = h / 2 - glass.level(1 - left) * h * 0.5;
      const size = Math.min(Math.max(Math.log2(p[PARAM.grainMs] / 10) / Math.log2(400), 0), 1);
      const r = 1 + 2 * size;
      ctx.fillStyle = c.ink;
      for (let g = 0; g + GRAIN_FIELDS <= f.grains.length; g += GRAIN_FIELDS) {
        const progress = f.grains[g + 1];
        const shape = f.grains[g + 2];
        const pan = f.grains[g + 3];
        const y = progress * pile;
        const spread = Math.min(glass.half(y) - r, w * 0.42);
        const x = (2 * pan - 1) * spread * Math.min(1, progress * 1.6);
        ctx.globalAlpha = 0.25 + 0.75 * windowAt(progress, shape);
        ctx.beginPath();
        ctx.arc(x, y, r, 0, 2 * Math.PI);
        ctx.fill();
      }
      ctx.globalAlpha = 1;
      // Clogged: grains waiting above the neck
      if (this.clogged) {
        ctx.fillStyle = c.ink;
        for (let k = 0; k < 9; k++) {
          const y = -DOT * (1 + Math.floor(k / 3));
          const x = ((k % 3) - 1) * DOT * 0.9 * (1 + Math.floor(k / 3) * 0.6);
          ctx.beginPath();
          ctx.arc(x, y, 1.4, 0, 2 * Math.PI);
          ctx.fill();
        }
      }
    }
    ctx.restore();
  }
}


// The glass's outline: two bulbs meeting at the neck. half(y) is the inside's
// half width at height y from the neck; level(k, neck) how high k of a bulb's
// sand reaches, 0..1 of its height, from its base or from the neck.
function outline(w: number, h: number, neck: number) {
  const half = (y: number) => {
    const t = Math.min(Math.abs(y) / (h / 2), 1);
    // Conical at the neck, widest at four fifths, a little in at the frame
    return neck / 2 + (w / 2 - neck / 2) * Math.sin(Math.PI * 0.62 * t) ** 0.85;
  };
  const path = new Path2D();
  const steps = 48;
  for (let i = 0; i <= steps; i++) {
    const y = -h / 2 + (i / steps) * h;
    if (i === 0) path.moveTo(-half(y), y);
    else path.lineTo(-half(y), y);
  }
  for (let i = steps; i >= 0; i--) {
    const y = -h / 2 + (i / steps) * h;
    path.lineTo(half(y), y);
  }
  path.closePath();
  // A bulb's area up to each height, from its base or from the neck
  const fromBase: number[] = [0];
  const fromNeck: number[] = [0];
  for (let i = 1; i <= steps; i++) {
    fromBase.push(fromBase[i - 1] + half(((h / 2) * (steps - i + 0.5)) / steps));
    fromNeck.push(fromNeck[i - 1] + half(((h / 2) * (i - 0.5)) / steps));
  }
  const level = (k: number, neck = false) => {
    const area = neck ? fromNeck : fromBase;
    const target = Math.min(Math.max(k, 0), 1) * area[steps];
    let i = 0;
    while (i < steps && area[i + 1] < target) i++;
    return i / steps;
  };
  return { path, half, level };
}

// Sand in a bulb, k of it, as dots on the grid: above, resting on the neck,
// below, from the base up
function sand(ctx: CanvasRenderingContext2D, glass: ReturnType<typeof outline>, h: number, k: number, top: boolean) {
  if (k <= 0) return;
  const reach = glass.level(k, top) * (h / 2);
  const [y0, y1] = top ? [-h / 2, 0] : [0, h / 2];
  for (let y = y0 + DOT / 2; y < y1; y += DOT) {
    if ((top ? -y : h / 2 - y) > reach) continue;
    const half = glass.half(y) - DOT;
    for (let x = -Math.floor(half / DOT) * DOT; x <= half; x += DOT) {
      ctx.beginPath();
      ctx.arc(x, y, 1.1, 0, 2 * Math.PI);
      ctx.fill();
    }
  }
}
