// The sound: Clessidra's firmware in an AudioWorklet (worklet.js runs clessidra.wasm,
// built by web/build.sh), and what plays into its input.

export class Audio {
  constructor(onView) {
    this.onView = onView
    this.ctx = null
    this.node = null
    this.input = null
    this.queue = [] // messages sent before the engine runs
  }

  get running() {
    return this.node !== null
  }

  get paused() {
    return this.ctx?.state === 'suspended'
  }

  // Everything stops where it is, and goes on from there
  async pause(on) {
    if (on) await this.ctx.suspend()
    else await this.ctx.resume()
  }

  send(m) {
    if (this.node) this.node.port.postMessage(m)
    else this.queue.push(m)
  }

  async start() {
    this.ctx = new AudioContext({ latencyHint: 'interactive' })
    const [bytes] = await Promise.all([
      fetch('clessidra.wasm').then((r) => r.arrayBuffer()),
      this.ctx.audioWorklet.addModule('worklet.js'),
    ])
    this.node = new AudioWorkletNode(this.ctx, 'clessidra', {
      numberOfInputs: 1,
      numberOfOutputs: 1,
      outputChannelCount: [2],
      processorOptions: { bytes },
    })
    this.node.port.onmessage = (e) => e.data.type === 'view' && this.onView(e.data)
    this.node.connect(this.ctx.destination)
    for (const m of this.queue) this.node.port.postMessage(m)
    this.queue = []
  }

  // 'demo', 'mic', or an audio File
  async useInput(kind) {
    if (!this.ctx) return
    this.input?.disconnect()
    this.input?.stop?.()
    this.input = null
    if (kind === 'demo') this.input = this.loop(demoTape(this.ctx))
    else if (kind === 'mic') {
      const stream = await navigator.mediaDevices.getUserMedia({
        audio: { echoCancellation: false, noiseSuppression: false, autoGainControl: false },
      })
      this.input = this.ctx.createMediaStreamSource(stream)
    } else if (kind instanceof File) {
      this.input = this.loop(await this.ctx.decodeAudioData(await kind.arrayBuffer()))
    }
    this.input?.connect(this.node)
  }

  loop(buffer) {
    const source = this.ctx.createBufferSource()
    source.buffer = buffer
    source.loop = true
    source.start()
    return source
  }
}

// A plucked phrase, made here (Karplus-Strong), so the demo needs no sound file
function demoTape(context) {
  const rate = context.sampleRate
  const notes = [57, 64, 69, 72, 71, 64, 67, 62] // MIDI
  const step = 0.5 // seconds
  const length = Math.ceil(notes.length * step * rate)
  const buffer = context.createBuffer(2, length, rate)
  const out = new Float32Array(length)
  notes.forEach((note, n) => {
    const period = Math.round(rate / (440 * 2 ** ((note - 69) / 12)))
    const line = Float32Array.from({ length: period }, () => Math.random() * 2 - 1)
    const begin = Math.round(n * step * rate)
    for (let i = 0, k = 0; begin + i < length && i < 2 * step * rate; i++) {
      const next = (k + 1) % period
      const s = 0.497 * (line[k] + line[next])
      line[k] = s
      k = next
      out[begin + i] += 0.5 * s
    }
  })
  buffer.copyToChannel(out, 0)
  buffer.copyToChannel(out, 1)
  return buffer
}
