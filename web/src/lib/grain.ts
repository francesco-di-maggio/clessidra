// The grain tile: one grain's window at its size in use, on a log axis from
// the shortest grain to the longest. Faint: the sizes and shapes randomness
// reaches. Below: the size and the pitch, a range when random.

import { PARAM } from "./engine";
import { windowAt } from "./waveform";

const RAND_PITCH = 24; // Core::kRandPitchSemitones

// How far randomness can move a value, as Core::Blend at its widest
const reach = (amount: number) => (amount >= 0 ? Math.min(amount, 1) : Math.min(-3 * amount, 1));
// The ends a 0..1 value reaches, as Core::Randomise
const ends = (x: number, amount: number) => [x - reach(amount) * x, x + reach(amount) * (1 - x)];

export class GrainView {
  private ctx: CanvasRenderingContext2D;
  private params: Float32Array = new Float32Array(0);

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => this.draw());
    new MutationObserver(() => this.draw()).observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme"] });
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
    const colour = (name: string) => css.getPropertyValue(name);

    // As the incoming strip: inset, the scale below
    const pad = 14;
    const axis = H - 12;
    const top = 8;
    const bottom = axis - 12;
    const width = W - 2 * pad;
    const min = p[PARAM.minGrainMs];
    const max = Math.max(p[PARAM.maxGrainMs], min * 1.01);
    const x = (ms: number) => pad + (Math.log(Math.max(ms, min) / min) / Math.log(max / min)) * width;

    // Size: the knob's 0..1 on the log range, as the core randomises it
    const set = p[PARAM.grainMsSet];
    const used = p[PARAM.grainMs];
    const octaves = Math.log2(max / min);
    const knob = Math.min(Math.max(Math.log2(set / min) / octaves, 0), 1);
    const [k0, k1] = p[PARAM.randSize] ? ends(knob, p[PARAM.randSize]) : [knob, knob];
    const scale = (k: number) => used * 2 ** ((k - knob) * octaves);
    const shape = p[PARAM.shape];
    const [s0, s1] = p[PARAM.randShape] ? ends(shape, p[PARAM.randShape]) : [shape, shape];

    const envelope = (ms: number, s: number) => {
      const right = x(ms);
      ctx.beginPath();
      ctx.moveTo(pad, bottom);
      for (let px = pad; px <= right; px++) ctx.lineTo(px, bottom - windowAt((px - pad) / Math.max(right - pad, 1), s) * (bottom - top));
      ctx.lineTo(right, bottom);
      ctx.stroke();
    };
    ctx.lineWidth = 1;
    ctx.strokeStyle = colour("--line");
    if (k0 !== k1 || s0 !== s1) for (const ms of new Set([scale(k0), scale(k1)])) for (const s of new Set([s0, s1])) envelope(ms, s);
    ctx.lineWidth = 1.5;
    ctx.strokeStyle = colour("--ink");
    envelope(used, shape);

    // Scale: a tick at each power of ten
    ctx.strokeStyle = colour("--line");
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(pad, axis - 7.5);
    ctx.lineTo(W - pad, axis - 7.5);
    for (let ms = 10; ms <= max; ms *= 10) {
      const px = Math.round(x(ms)) + 0.5;
      ctx.moveTo(px, axis - 7.5);
      ctx.lineTo(px, axis - 3.5);
    }
    ctx.stroke();

    // The size and the pitch, a range where random
    const ms = (v: number) => (v < 1000 ? `${Math.round(v)} ms` : `${(v / 1000).toFixed(1)} s`);
    const st = (v: number) => `${v > 0 ? "+" : v < 0 ? "−" : ""}${Math.abs(v).toFixed(Math.abs(v) < 1 && v !== 0 ? 1 : 0)}`;
    const pitch = p[PARAM.pitch];
    const spread = p[PARAM.randPitch] ? reach(p[PARAM.randPitch]) * RAND_PITCH : 0;
    ctx.font = `500 10px ${colour("--mono")}`;
    ctx.textBaseline = "middle";
    ctx.fillStyle = colour("--dim");
    ctx.textAlign = "left";
    ctx.fillText(k0 !== k1 ? `${ms(scale(k0))}–${ms(scale(k1))}` : ms(used), pad, axis + 3);
    ctx.textAlign = "right";
    ctx.fillText(spread ? `${st(pitch - spread)} to ${st(pitch + spread)} st` : `${st(pitch)} st`, W - pad, axis + 3);
  }
}
