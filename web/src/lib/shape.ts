// One grain of sand between Position and Length, as the knobs set it: as big
// as its Size (5 ms to 2 s), as high as its Pitch (two octaves either way),
// leaning as its Shape: round at Hann, heavy at its start when percussive, at
// its end when reversed. Random size and random pitch: how far it may go,
// faint.

import { PARAM } from "./engine";

const SIZE_MIN = 5; // ms, CoreUI::kMinGrainMs
const SIZE_MAX = 2000; // ms, CoreUI::kMaxGrainMs
const PITCH_RANGE = 24; // semitones, either way
const RAND_PITCH = 12; // semitones, Core::kRandPitchSemitones
const SMALLEST = 3; // px, the radius at the shortest size
const LARGEST = 0.6; // of half the height, at the longest: the rest is Pitch's
const LEAN = 0.55; // how much of its width a grain loses at its light end
const STEPS = 72; // round it

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
    const shown = [PARAM.grainMs, PARAM.shape, PARAM.pitch, PARAM.randSize, PARAM.randPitch, PARAM.minGrainMs, PARAM.maxGrainMs];
    const same = this.params.length === params.length && shown.every((i) => this.params[i] === params[i]);
    this.params = params.slice();
    if (!same) this.draw();
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

    const half = H / 2 - 2;
    const largest = LARGEST * half;
    const radius = (ms: number) => SMALLEST + (largest - SMALLEST) * Math.min(1, Math.max(0, Math.log(ms / SIZE_MIN) / Math.log(SIZE_MAX / SIZE_MIN)));
    const height = (semitones: number) => H / 2 - Math.max(-1, Math.min(1, semitones / PITCH_RANGE)) * (half - largest);
    // -1 reversed .. 0 round .. 1 percussive: heavy where the sound is
    const lean = 2 * Math.min(Math.max(p[PARAM.shape], 0), 1) - 1;
    const grain = (r: number, y: number, colour: string, solid: boolean) => {
      ctx.beginPath();
      for (let k = 0; k <= STEPS; k++) {
        const a = (2 * Math.PI * k) / STEPS;
        // Longer as it leans, narrower towards its light end
        const x = Math.cos(a) * r * (1 + 0.5 * Math.abs(lean));
        const light = 0.5 + 0.5 * Math.sign(lean) * Math.cos(a); // 1 at its light end
        ctx.lineTo(W / 2 + x, y + Math.sin(a) * r * (1 - LEAN * Math.abs(lean) * light));
      }
      ctx.closePath();
      ctx.fillStyle = ctx.strokeStyle = colour;
      ctx.lineWidth = 1;
      if (solid) ctx.fill();
      ctx.stroke();
    };

    const ms = p[PARAM.grainMs];
    const pitch = p[PARAM.pitch];
    const faint = css.getPropertyValue("--line");
    const octaves = Math.log2(Math.max(p[PARAM.maxGrainMs], p[PARAM.minGrainMs] * 1.01) / p[PARAM.minGrainMs]);
    const reach = Math.min(Math.max(Math.sqrt(4 * p[PARAM.randSize]), 0), 1) * 0.25 * octaves;
    if (reach > 0) grain(radius(ms * 2 ** reach), height(pitch), faint, false);
    // In octaves, below 0: an octave either way whatever the odds
    const spread = p[PARAM.randPitch] < 0 ? RAND_PITCH : p[PARAM.randPitch] * RAND_PITCH;
    if (spread > 0) for (const s of [-spread, spread]) grain(radius(ms), height(pitch + s), faint, false);
    grain(radius(ms), height(pitch), css.getPropertyValue("--accent"), true);
  }
}
