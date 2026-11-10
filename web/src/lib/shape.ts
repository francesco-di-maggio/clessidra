// The grain at the playhead, between Position and Length: the recording as
// that grain reads it, as wide as its Size (5 ms to 2 s across), as fast as
// its Pitch, swelling and fading as Shape sets. Nothing recorded: the shape
// alone. Random size: the narrowest and widest shapes, faint.

import { PARAM, WAVE_COLUMNS, type Frame } from "./engine";
import { windowAt } from "./waveform";

const SIZE_MIN = 5; // ms, CoreUI::kMinGrainMs
const SIZE_MAX = 2000; // ms, CoreUI::kMaxGrainMs
const INSET = 8; // px, at the sides
const STEPS = 96; // along a shape
const FLOOR = 0.05; // of full scale: a quiet recording is not drawn tall

export class GrainShape {
  private ctx: CanvasRenderingContext2D;
  private params: Float32Array = new Float32Array(0);
  private wave: Float32Array = new Float32Array(0);
  private level = 0; // the recording's peak

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => this.draw());
    new MutationObserver(() => this.draw()).observe(document.documentElement, { attributes: true, attributeFilter: ["data-theme"] });
    new ResizeObserver(() => this.draw()).observe(canvas);
  }

  update(m: Frame) {
    // Drawn again only when what it shows changes
    const shown = [PARAM.grainMs, PARAM.shape, PARAM.randSize, PARAM.minGrainMs, PARAM.maxGrainMs];
    const same = this.params.length === m.params.length && shown.every((i) => this.params[i] === m.params[i])
      && this.wave.length === m.wave.length && this.wave.every((x, i) => x === m.wave[i]);
    this.params = m.params.slice();
    this.wave = m.wave.slice();
    this.level = m.peaks.reduce((a, b) => Math.max(a, b), 0);
    if (!same) this.draw();
  }

  draw() {
    const { canvas, ctx, params: p, wave } = this;
    const dpr = devicePixelRatio || 1;
    const W = canvas.clientWidth;
    const H = canvas.clientHeight;
    if (!W || !H || p.length <= PARAM.maxGrainMs) return;
    if (canvas.width !== Math.round(W * dpr)) canvas.width = Math.round(W * dpr);
    if (canvas.height !== Math.round(H * dpr)) canvas.height = Math.round(H * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const css = getComputedStyle(document.documentElement);
    const accent = css.getPropertyValue("--accent");

    const shape = p[PARAM.shape];
    const mid = H / 2;
    const room = W - 2 * INSET;
    const amplitude = mid - 6;
    const width = (ms: number) => (0.08 + 0.92 * Math.min(1, Math.max(0, Math.log(ms / SIZE_MIN) / Math.log(SIZE_MAX / SIZE_MIN)))) * room;
    // The window alone, above and below the middle
    const outline = (w: number, colour: string, lineWidth: number) => {
      const x0 = (W - w) / 2;
      ctx.beginPath();
      for (const side of [-1, 1]) {
        for (let k = 0; k <= STEPS; k++) {
          const x = k / STEPS;
          const y = mid + side * windowAt(x, shape) * amplitude;
          if (k === 0) ctx.moveTo(x0, y);
          else ctx.lineTo(x0 + x * w, y);
        }
      }
      ctx.strokeStyle = colour;
      ctx.lineWidth = lineWidth;
      ctx.stroke();
    };

    const ms = p[PARAM.grainMs];
    const octaves = Math.log2(Math.max(p[PARAM.maxGrainMs], p[PARAM.minGrainMs] * 1.01) / p[PARAM.minGrainMs]);
    const reach = Math.min(Math.max(Math.sqrt(4 * p[PARAM.randSize]), 0), 1) * 0.25 * octaves;
    if (reach > 0) for (const r of [-reach, reach]) outline(width(ms * 2 ** r), css.getPropertyValue("--line"), 1);

    const w = width(ms);
    if (wave.length < 2 * WAVE_COLUMNS || this.level === 0) {
      outline(w, accent, 1.5);
      return;
    }
    // Low to high in each column, under the window
    const x0 = (W - w) / 2;
    const scale = amplitude / Math.max(this.level, FLOOR);
    const at = (c: number, edge: number) => mid - wave[2 * c + edge] * windowAt(c / (WAVE_COLUMNS - 1), shape) * scale;
    ctx.beginPath();
    for (let c = 0; c < WAVE_COLUMNS; c++) ctx.lineTo(x0 + (c / (WAVE_COLUMNS - 1)) * w, at(c, 1));
    for (let c = WAVE_COLUMNS - 1; c >= 0; c--) ctx.lineTo(x0 + (c / (WAVE_COLUMNS - 1)) * w, at(c, 0));
    ctx.closePath();
    ctx.fillStyle = accent;
    ctx.strokeStyle = accent;
    ctx.lineWidth = 1.5;
    ctx.lineJoin = "round";
    ctx.fill();
    ctx.stroke();
  }
}
