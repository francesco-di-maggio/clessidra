// The sound: the firmware in an AudioWorklet (worklet.ts), and what plays into
// its input, as into the board's.

import { RATE, type Control, type FromWorklet } from "./engine";
import { url } from "./url";
import workletUrl from "./worklet.ts?worker&url";

export type Input = "demo" | "mic" | File;

export class Audio {
  ctx: AudioContext | null = null;
  private node: AudioWorkletNode | null = null;
  private input: AudioNode | null = null;
  private stream: MediaStream | null = null;
  private queue: Control[] = []; // controls set before the firmware runs
  // Browser only, the board has its own: the level into the firmware and out
  private inGain: GainNode | null = null;
  private outGain: GainNode | null = null;
  private gains = { input: 1, output: 1 };

  constructor(private onMessage: (m: FromWorklet) => void) {}

  send(m: Control) {
    if (this.node) this.node.port.postMessage(m);
    else this.queue.push(m);
  }

  // The board's rate: the mapping counts in 48 kHz. The browser resamples to
  // the device.
  async start() {
    const ctx = (this.ctx = new AudioContext({ sampleRate: RATE, latencyHint: "interactive" }));
    const [bytes] = await Promise.all([
      fetch(url("clessidra.wasm")).then((r) => r.arrayBuffer()),
      ctx.audioWorklet.addModule(workletUrl),
    ]);
    this.node = new AudioWorkletNode(ctx, "clessidra", {
      numberOfInputs: 1,
      numberOfOutputs: 1,
      outputChannelCount: [2],
      processorOptions: { bytes },
    });
    this.node.port.onmessage = (e: MessageEvent<FromWorklet>) => this.onMessage(e.data);
    this.inGain = new GainNode(ctx, { gain: this.gains.input });
    this.outGain = new GainNode(ctx, { gain: this.gains.output });
    this.inGain.connect(this.node);
    this.node.connect(this.outGain).connect(ctx.destination);
    for (const m of this.queue) this.node.port.postMessage(m);
    this.queue = [];
  }

  get running() {
    return this.ctx?.state === "running";
  }

  async pause(on: boolean) {
    if (on) await this.ctx?.suspend();
    else await this.ctx?.resume();
  }

  async use(kind: Input) {
    const ctx = this.ctx!;
    this.input?.disconnect();
    if (this.input instanceof AudioBufferSourceNode) this.input.stop();
    this.stream?.getTracks().forEach((t) => t.stop());
    this.input = this.stream = null;
    if (kind === "demo") this.input = loop(ctx, demo(ctx));
    else if (kind === "mic") {
      this.stream = await navigator.mediaDevices.getUserMedia({
        audio: { echoCancellation: false, noiseSuppression: false, autoGainControl: false },
      });
      this.input = ctx.createMediaStreamSource(this.stream);
    } else this.input = loop(ctx, await ctx.decodeAudioData(await kind.arrayBuffer()));
    this.input.connect(this.inGain!);
  }

  // Linear gains, smoothed over 20 ms
  setGain(which: "input" | "output", gain: number) {
    this.gains[which] = gain;
    const node = which === "input" ? this.inGain : this.outGain;
    node?.gain.setTargetAtTime(gain, this.ctx!.currentTime, 0.02);
  }
}

function loop(ctx: AudioContext, buffer: AudioBuffer) {
  const source = ctx.createBufferSource();
  source.buffer = buffer;
  source.loop = true;
  source.start();
  return source;
}

// A plucked phrase, made here (Karplus-Strong): the demo needs no sound file.
// Notes of different lengths and levels, so the slices find onsets.
function demo(ctx: AudioContext) {
  const rate = ctx.sampleRate;
  const notes: [number, number, number][] = [
    [57, 0.5, 0.6], [64, 0.25, 0.4], [69, 0.25, 0.5], [72, 1, 0.7],
    [71, 0.5, 0.5], [64, 0.5, 0.4], [67, 0.75, 0.6], [62, 1.25, 0.7],
  ]; // MIDI note, seconds, level
  const length = Math.ceil(notes.reduce((t, [, s]) => t + s, 0) * rate);
  const out = new Float32Array(length);
  let begin = 0;
  for (const [note, step, level] of notes) {
    const period = Math.round(rate / (440 * 2 ** ((note - 69) / 12)));
    const line = Float32Array.from({ length: period }, () => Math.random() * 2 - 1);
    for (let i = 0, k = 0; begin + i < length && i < 2 * rate; i++) {
      const next = (k + 1) % period;
      line[k] = 0.498 * (line[k] + line[next]);
      out[begin + i] += level * line[k];
      k = next;
    }
    begin += Math.round(step * rate);
  }
  const buffer = ctx.createBuffer(1, length, rate);
  buffer.copyToChannel(out, 0);
  return buffer;
}
