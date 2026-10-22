// Host test: the web page's worklet (docs/worklet.js) with the real WebAssembly
// (docs/clessidra.wasm), driven like the page drives it. Run by tests/run.sh.

import { readFileSync } from 'node:fs'
import { fileURLToPath } from 'node:url'

const docs = fileURLToPath(new URL('../docs/', import.meta.url))
let failures = 0
const check = (ok, name, value) => {
  console.log(`${ok ? 'PASS' : 'FAIL'}  ${name.padEnd(52)} ${value}`)
  if (!ok) failures++
}

// Stand-ins for the AudioWorklet scope
globalThis.sampleRate = 48000
globalThis.currentTime = 0
let Processor
globalThis.AudioWorkletProcessor = class {
  constructor() {
    this.port = { postMessage: (m) => posted.push(m), onmessage: null }
  }
}
globalThis.registerProcessor = (_, cls) => (Processor = cls)
const posted = []
await import(docs + 'worklet.js')

const p = new Processor({ processorOptions: { bytes: readFileSync(docs + 'clessidra.wasm').buffer } })
const send = (m) => p.port.onmessage({ data: m })
;[0, 0.5, 0.1, 0.5, 0.2, 1, 0.5, 0.8].forEach((value, index) => send({ type: 'knob', index, value }))
send({ type: 'switch', index: 0, position: 1 })
send({ type: 'second', index: 5, value: 0.2 })
await new Promise((r) => setTimeout(r, 200)) // instantiate

const input = new Float32Array(128)
const out = [new Float32Array(128), new Float32Array(128)]
let n = 0
let energy = 0
let count = 0
for (let b = 0; b < 1500; b++) {
  globalThis.currentTime = (b * 128) / sampleRate
  if (b === 50) send({ type: 'pad', index: 10, touched: true, pressure: 0 })
  if (b === 425) send({ type: 'pad', index: 10, touched: false, pressure: 0 })
  for (let i = 0; i < 128; i++) input[i] = 0.3 * Math.sin((2 * Math.PI * 220 * n++) / sampleRate)
  p.process([[input, input]], [out])
  if (b > 900) for (let i = 0; i < 128; i++, count++) energy += out[0][i] ** 2
}

const views = posted.filter((m) => m.type === 'view')
const last = views.at(-1)
const recording = views.filter((m) => m.outline).at(-1)
check(posted.some((m) => m.type === 'ready'), 'engine ready', true)
check(views.length > 100, 'display updates, about 30 a second', views.length)
check(Math.abs(recording.seconds - 1.05) < 0.03, 'P10 held 1 s: recording (s)', recording.seconds.toFixed(3))
check(Math.abs(Math.max(...recording.outline) - 0.3) < 0.02, 'recording outline peak (input 0.3)', Math.max(...recording.outline).toFixed(3))
check(last.grains.length > 0, 'grains playing', last.grains.length / 3)
check(Math.abs(last.second[5] - 0.2) < 1e-6, 'second layer set and reported back', last.second[5].toFixed(3))
check(Math.sqrt(energy / count) > 0.05, 'sound at the output (rms)', Math.sqrt(energy / count).toFixed(3))
console.log(failures ? 'Some checks failed.' : 'All checks passed.')
process.exit(failures ? 1 : 0)
