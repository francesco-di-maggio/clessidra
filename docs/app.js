// Clessidra in the browser: the hourglass view changes the controls, which go to the
// firmware running in an AudioWorklet (audio.js).

import { Audio } from './audio.js'
import { Controls, PAD, PLACE_PADS } from './controls.js'
import { buildGlass } from './glass.js'

const params = new URLSearchParams(location.search)
if (params.has('embed')) document.documentElement.dataset.embed = ''

const readout = document.querySelector('[data-readout]')
const startButton = document.querySelector('[data-start]')
const fileInput = document.querySelector('[data-file]')
const led = document.querySelector('[data-led]')

let view = {
  led: 0, grains: new Float32Array(0), sources: new Float32Array(20),
  start: 0, length: 1, outline: new Float32Array(0), seconds: 0, running: false,
}
const audio = new Audio((m) => {
  const outline = m.outline ?? view.outline
  const seconds = m.seconds ?? view.seconds
  view = { ...m, outline, seconds, running: true }
  // The second layer as the instrument holds it
  if (performance.now() - controls.secondEdited > 300) m.second.forEach((v, k) => controls.second[k] !== null && (controls.second[k] = v))
})
const controls = new Controls((m) => audio.send(m))
controls.sync()
// ?debug: the page's state, for inspecting in the console
if (params.has('debug')) window.clessidra = { audio, controls, view: () => view }

const drawGlass = buildGlass(document.querySelector('[data-glass]'), controls, readout)

// Start and input
let inputKind = 'demo'
async function useInput(kind) {
  inputKind = kind
  for (const b of document.querySelectorAll('[data-input]')) b.setAttribute('aria-checked', String(b.dataset.input === kind))
  if (kind === 'file') fileInput.click()
  else await audio.useInput(kind)
}
// Start, then Pause and Play: all sound stops and goes on from where it was
startButton.addEventListener('click', async () => {
  if (audio.running) {
    await audio.pause(!audio.paused)
    startButton.textContent = audio.paused ? 'Play' : 'Pause'
    return
  }
  startButton.disabled = true
  startButton.textContent = 'Loading'
  try {
    await audio.start()
    await useInput(inputKind)
    startButton.textContent = 'Pause'
  } catch (err) {
    startButton.textContent = 'Start'
    readout.textContent = `Could not start: ${err.message}`
  }
  startButton.disabled = false
})
for (const b of document.querySelectorAll('[data-input]')) {
  b.addEventListener('click', () => useInput(b.dataset.input).catch((err) => (readout.textContent = err.message)))
}
fileInput.addEventListener('change', () => fileInput.files?.[0] && audio.useInput(fileInput.files[0]))

// Keys: Space record, Shift second layer, 1-7 the places, 0 stop
const KEYS = { Space: PAD.record, ShiftLeft: PAD.shift, ShiftRight: PAD.shift, Digit0: PAD.stop }
PLACE_PADS.forEach((pad, place) => (KEYS[`Digit${place + 1}`] = pad))
addEventListener('keydown', (e) => {
  const pad = KEYS[e.code]
  if (pad === undefined || e.repeat || e.target instanceof HTMLInputElement) return
  e.preventDefault()
  controls.press(pad, `k${e.code}`)
})
addEventListener('keyup', (e) => controls.release(`k${e.code}`))
addEventListener('blur', () => controls.releaseAll())

// Every frame
const css = getComputedStyle(document.documentElement)
const colour = (name) => css.getPropertyValue(name).trim()
function frame() {
  controls.tick()
  const colours = {
    ink: colour('--ink'), line: colour('--line'), dim: colour('--dim'), accent: colour('--accent'), mono: colour('--mono'),
    sand: colour('--sand'), sandDeep: colour('--sand-deep'), glass: colour('--glass'), glassEdge: colour('--glass-edge'),
    shine: colour('--shine'), veil: colour('--veil'), dark: colour('--dark') === '1',
  }
  drawGlass(view, colours)
  led.style.opacity = view.led
  requestAnimationFrame(frame)
}
requestAnimationFrame(frame)
