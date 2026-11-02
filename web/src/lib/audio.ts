// The sound: the firmware in an AudioWorklet (worklet.js), and what plays into
// its input, as into the board's.

export class Audio {
  constructor(onMessage) {
    this.onMessage = onMessage;
    this.ctx = null;
    this.node = null;
    this.input = null;
    this.stream = null;
    this.queue = []; // controls set before the firmware runs
  }

  send(m) {
    if (this.node) this.node.port.postMessage(m);
    else this.queue.push(m);
  }

  // The board's rate: the mapping counts in 48 kHz. The browser resamples to
  // the device.
  async start() {
    this.ctx = new AudioContext({ sampleRate: 48000, latencyHint: "interactive" });
    const base = new URL("../", import.meta.url);
    const [bytes] = await Promise.all([
      fetch(new URL("clessidra.wasm", base)).then((r) => r.arrayBuffer()),
      this.ctx.audioWorklet.addModule(new URL("js/worklet.js", base)),
    ]);
    this.node = new AudioWorkletNode(this.ctx, "clessidra", {
      numberOfInputs: 1,
      numberOfOutputs: 1,
      outputChannelCount: [2],
      processorOptions: { bytes },
    });
    this.node.port.onmessage = (e) => this.onMessage(e.data);
    this.node.connect(this.ctx.destination);
    for (const m of this.queue) this.node.port.postMessage(m);
    this.queue = [];
  }

  get running() {
    return this.ctx?.state === "running";
  }

  async pause(on) {
    if (on) await this.ctx.suspend();
    else await this.ctx.resume();
  }

  // "demo", "mic", or an audio File
  async use(kind) {
    this.input?.disconnect();
    this.input?.stop?.();
    this.stream?.getTracks().forEach((t) => t.stop());
    this.input = this.stream = null;
    if (kind === "demo") this.input = this.loop(demo(this.ctx));
    else if (kind === "mic") {
      this.stream = await navigator.mediaDevices.getUserMedia({
        audio: { echoCancellation: false, noiseSuppression: false, autoGainControl: false },
      });
      this.input = this.ctx.createMediaStreamSource(this.stream);
    } else if (kind instanceof File) {
      this.input = this.loop(await this.ctx.decodeAudioData(await kind.arrayBuffer()));
    }
    this.input?.connect(this.node);
  }

  loop(buffer) {
    const source = this.ctx.createBufferSource();
    source.buffer = buffer;
    source.loop = true;
    source.start();
    return source;
  }
}

// A plucked phrase, made here (Karplus-Strong): the demo needs no sound file.
// Notes of different lengths and levels, so the slices find onsets.
function demo(ctx) {
  const rate = ctx.sampleRate;
  const notes = [
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
