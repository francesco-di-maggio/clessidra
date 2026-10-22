// The engine on the audio thread: Clessidra's firmware, compiled to WebAssembly
// (web/build.sh), runs here block by block. The page sends the controls and gets
// back what the display shows.

const VIEW_EVERY = 1 / 60 // seconds between display updates: the screen's rate

class ClessidraProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super()
    this.wasm = null
    this.recordingKey = ''
    this.nextView = 0
    this.port.onmessage = (e) => this.onMessage(e.data)
    this.load(options.processorOptions.bytes)
  }

  async load(bytes) {
    // A standalone module: any import it asks for is a stub that returns 0
    const stub = new Proxy({}, { get: () => () => 0 })
    const imports = new Proxy({}, { get: () => stub })
    const { instance } = await WebAssembly.instantiate(bytes, imports)
    const w = instance.exports
    w._initialize?.()
    w.cl_init(sampleRate)
    const f32 = (ptr, n) => new Float32Array(w.memory.buffer, ptr, n)
    this.io = {
      inL: f32(w.cl_input(0), 128), inR: f32(w.cl_input(1), 128),
      outL: f32(w.cl_output(0), 128), outR: f32(w.cl_output(1), 128),
    }
    this.wasm = w
    this.f32 = f32
    for (const m of this.pending ?? []) this.onMessage(m)
    this.pending = null
    this.port.postMessage({ type: 'ready' })
  }

  onMessage(m) {
    if (!this.wasm) {
      ;(this.pending ??= []).push(m)
      return
    }
    const w = this.wasm
    if (m.type === 'knob') w.cl_knob(m.index, m.value)
    else if (m.type === 'second') w.cl_second_layer(m.index, m.value)
    else if (m.type === 'switch') w.cl_switch(m.index, m.position)
    else if (m.type === 'pad') w.cl_pad(m.index, m.touched ? 1 : 0, m.pressure)
  }

  process(inputs, outputs) {
    const out = outputs[0]
    if (!this.wasm) return true
    const n = out[0].length
    const input = inputs[0]
    const inL = input?.[0], inR = input?.[1] ?? inL
    if (inL) {
      this.io.inL.set(inL)
      this.io.inR.set(inR)
    } else {
      this.io.inL.fill(0)
      this.io.inR.fill(0)
    }
    const now = Math.floor(currentTime * 1000)
    this.wasm.cl_process(n, now)
    out[0].set(this.io.outL.subarray(0, n))
    if (out[1]) out[1].set(this.io.outR.subarray(0, n))
    if (currentTime >= this.nextView) {
      this.nextView = currentTime + VIEW_EVERY
      this.sendView(now)
    }
    return true
  }

  // What the display draws, about 30 times a second
  sendView(now) {
    const w = this.wasm
    const count = w.cl_grains()
    const grains = this.f32(w.cl_grain_view(), count * 3).slice()
    const sources = new Float32Array(20)
    for (let s = 0; s < 10; s++) {
      sources[s * 2] = w.cl_source_phase(s)
      sources[s * 2 + 1] = w.cl_source_on(s)
    }
    const view = {
      type: 'view',
      led: w.cl_led(now),
      grains,
      sources,
      start: w.cl_selection_start(),
      length: w.cl_selection_length(),
      second: Array.from({ length: 8 }, (_, k) => w.cl_second_layer_value(k)),
    }
    // The recording's outline, only when it changed
    const length = w.cl_recording_length()
    const key = `${w.cl_recording()}:${length}`
    if (key !== this.recordingKey) {
      this.recordingKey = key
      view.outline = length ? outline(this.f32(w.cl_recording(), length), 600) : new Float32Array(0)
      view.seconds = length / sampleRate
    }
    this.port.postMessage(view, [grains.buffer, sources.buffer])
  }
}

// Peak level per column
function outline(samples, columns) {
  const peaks = new Float32Array(columns)
  if (!samples.length) return peaks
  const per = samples.length / columns
  for (let c = 0; c < columns; c++) {
    let peak = 0
    const end = Math.min(samples.length, Math.floor((c + 1) * per))
    for (let i = Math.floor(c * per); i < end; i++) peak = Math.max(peak, Math.abs(samples[i]))
    peaks[c] = peak
  }
  return peaks
}

registerProcessor('clessidra', ClessidraProcessor)
