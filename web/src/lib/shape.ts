// One grain between Position and Length, as the knobs set it: its sound only,
// as wide as its Size (5 ms to 2 s across), swelling and fading as Shape sets,
// as fast as its Pitch. Random size: the narrowest and widest, faint. Random
// pitch: the lowest and highest, faint.

import { PARAM } from "./engine";
import { windowAt } from "./waveform";

const SIZE_MIN = 5; // ms, CoreUI::kMinGrainMs
const SIZE_MAX = 2000; // ms, CoreUI::kMaxGrainMs
const CYCLES = 6; // inside the grain at unison
const RAND_PITCH = 12; // semitones, Core::kRandPitchSemitones
const INSET = 8; // px, at the sides

export class GrainShape {
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
    const shown = [PARAM.grainMs, PARAM.shape, PARAM.pitch, PARAM.randSize, PARAM.randPitch];
    const same = this.params.length === params.length && shown.every((i) => this.params[i] === params[i]);
    this.params = params.slice();
    if (!same) this.draw();
  }

  draw() {
    const { canvas, ctx, params: p } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H || p.length <= PARAM.randPitch) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);

    const shape = p[PARAM.shape];
    const mid = H / 2;
    const room = W - 2 * INSET;
    const amplitude = mid - 6; // a little in from the dial and its value
    const width = (ms: number) => (0.08 + 0.92 * Math.min(1, Math.max(0, Math.log(ms / SIZE_MIN) / Math.log(SIZE_MAX / SIZE_MIN)))) * room;
    const grain = (w: number, semitones: number, colour: string, lineWidth: number) => {
      const cycles = CYCLES * 2 ** (semitones / 12);
      const x0 = (W - w) / 2;
      const steps = Math.max(96, Math.ceil(2 * w));
      ctx.beginPath();
      for (let k = 0; k <= steps; k++) {
        const x = k / steps;
        ctx.lineTo(x0 + x * w, mid - Math.sin(2 * Math.PI * cycles * x) * windowAt(x, shape) * amplitude);
      }
      ctx.strokeStyle = colour;
      ctx.lineWidth = lineWidth;
      ctx.stroke();
    };

    const ms = p[PARAM.grainMs];
    const pitch = p[PARAM.pitch];
    const faint = css.getPropertyValue("--line");
    const octaves = Math.log2(Math.max(p[PARAM.maxGrainMs], p[PARAM.minGrainMs] * 1.01) / p[PARAM.minGrainMs]);
    const reach = Math.min(Math.max(Math.sqrt(4 * p[PARAM.randSize]), 0), 1) * 0.25 * octaves;
    if (reach > 0) for (const r of [-reach, reach]) grain(width(ms * 2 ** r), pitch, faint, 1);
    const spread = p[PARAM.randPitch] * RAND_PITCH;
    if (spread > 0) for (const s of [-spread, spread]) grain(width(ms), pitch + s, faint, 1);
    grain(width(ms), pitch, css.getPropertyValue("--accent"), 1.5);
  }
}
