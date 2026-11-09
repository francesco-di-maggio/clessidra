// One grain between Position and Length, as the knobs set it: as wide as its
// Size (5 ms to 2 s across), its envelope Shape's, the sound inside it as
// fast as its Pitch. Random size: the widths it may take, faint. Random pitch:
// the sound inside drawn faint beside the highest and lowest.

import { PARAM } from "./engine";
import { windowAt } from "./waveform";

const SIZE_MIN = 5; // ms, CoreUI::kMinGrainMs
const SIZE_MAX = 2000; // ms, CoreUI::kMaxGrainMs
const CYCLES = 6; // inside the grain at unison
const RAND_PITCH = 12; // semitones, Core::kRandPitchSemitones

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
    const p = params;
    const same = this.params.length === p.length && [PARAM.grainMs, PARAM.shape, PARAM.pitch, PARAM.randSize, PARAM.randPitch].every((i) => this.params[i] === p[i]);
    this.params = p.slice();
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
    const ink = css.getPropertyValue("--ink");
    const dim = css.getPropertyValue("--dim");
    const line = css.getPropertyValue("--line");

    const shape = p[PARAM.shape];
    const floor = H - 1;
    const mid = H / 2;
    const width = (ms: number) => (0.1 + 0.9 * Math.min(1, Math.max(0, Math.log(ms / SIZE_MIN) / Math.log(SIZE_MAX / SIZE_MIN)))) * W;
    const envelope = (w: number) => {
      ctx.beginPath();
      for (let k = 0; k <= 64; k++) ctx.lineTo((W - w) / 2 + (k / 64) * w, floor - windowAt(k / 64, shape) * (H - 2));
    };

    // Random size: the narrowest and widest it may take
    const octaves = Math.log2(Math.max(p[PARAM.maxGrainMs], p[PARAM.minGrainMs] * 1.01) / p[PARAM.minGrainMs]);
    const reach = Math.min(Math.max(Math.sqrt(4 * p[PARAM.randSize]), 0), 1) * 0.25 * octaves;
    const ms = p[PARAM.grainMs];
    ctx.lineWidth = 1;
    ctx.strokeStyle = line;
    if (reach > 0) for (const w of [width(ms * 2 ** -reach), width(ms * 2 ** reach)]) (envelope(w), ctx.stroke());

    // The sound inside, as fast as the pitch; with random pitch, its ends
    const w = width(ms);
    const x0 = (W - w) / 2;
    const inside = (semitones: number, colour: string) => {
      const cycles = CYCLES * 2 ** (semitones / 12);
      ctx.beginPath();
      const steps = Math.max(64, Math.ceil(w));
      for (let k = 0; k <= steps; k++) {
        const x = k / steps;
        ctx.lineTo(x0 + x * w, mid - Math.sin(2 * Math.PI * cycles * x) * windowAt(x, shape) * (mid - 2));
      }
      ctx.strokeStyle = colour;
      ctx.stroke();
    };
    const pitch = p[PARAM.pitch];
    const spread = p[PARAM.randPitch] * RAND_PITCH;
    if (spread > 0) for (const s of [pitch - spread, pitch + spread]) inside(s, line);
    inside(pitch, dim);

    // The envelope
    envelope(w);
    ctx.strokeStyle = ink;
    ctx.lineWidth = 1.5;
    ctx.stroke();
  }
}
