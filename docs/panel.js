// The panel view: the Simple Touch as it is, every control where it sits on the
// faceplate. P11 switches the knobs to their second layer, as on the board.

import { PAD, PLACE_PADS, KNOB_HELP, SWITCH_HELP, clamp } from './controls.js'
import { svg, describe } from './ui.js'

const DRAG_PX = 200
const SWEEP = 270

// Positions in the viewBox, as on the faceplate. Knob and fader index = Controls::Knob.
const KNOBS = [
  { index: 0, code: 'S30', x: 150, y: 330 },
  { index: 1, code: 'S31', x: 150, y: 200 },
  { index: 2, code: 'S32', x: 250, y: 200 },
  { index: 3, code: 'S33', x: 350, y: 200 },
  { index: 4, code: 'S34', x: 450, y: 200 },
  { index: 5, code: 'S35', x: 450, y: 330 },
]
const FADERS = [
  { index: 6, code: 'S36', x: 56, y0: 150, y1: 400 },
  { index: 7, code: 'S37', x: 544, y0: 150, y1: 400 },
]
const SWITCHES = [
  { index: 0, code: 'S09', x: 245, y: 520, side: -1 },
  { index: 1, code: 'S07', x: 355, y: 565, side: 1 },
]
const PADS = [
  { x: 150, y: 500 }, { x: 300, y: 470 }, { x: 455, y: 500 }, { x: 60, y: 650 },
  { x: 175, y: 630 }, { x: 300, y: 665 }, { x: 430, y: 645 }, { x: 545, y: 695 },
  { x: 200, y: 775 }, { x: 420, y: 780 }, { x: 225, y: 70 }, { x: 375, y: 70 },
]
const SHORT = ['Start', 'Size', 'Random position', 'Pitch', 'Random pitch', 'End', 'Speed', 'Flow']
const SHORT_SECOND = [null, 'Window', 'Spread', null, 'Reverb', 'Tone', 'Dry/wet', 'Resample']

function padName(i) {
  if (i === PAD.stop) return 'Stop'
  if (i === PAD.record) return 'Record'
  if (i === PAD.shift) return 'Second layer'
  return `Place ${PLACE_PADS.indexOf(i) + 1}`
}

export function buildPanel(board, controls, readout) {
  const draws = []
  const point = (e) => {
    const pt = board.createSVGPoint()
    pt.x = e.clientX
    pt.y = e.clientY
    return pt.matrixTransform(board.getScreenCTM().inverse())
  }
  const knobText = (k) => {
    const [main, second] = KNOB_HELP[k.index]
    return `${k.code} · ${controls.shifted ? (second ?? 'no second layer') : main}`
  }

  for (const k of KNOBS) {
    const g = svg('g', { class: 'control', tabindex: 0, 'aria-label': k.code }, board)
    svg('circle', { class: 'shape', cx: k.x, cy: k.y, r: 34 }, g)
    const mark = svg('line', { class: 'mark' }, g)
    svg('text', { class: 'code', x: k.x, y: k.y + 4 }, g).textContent = k.code
    const label = svg('text', { x: k.x, y: k.y + 54 }, g)
    let start = 0
    let y0 = 0
    g.addEventListener('pointerdown', (e) => {
      g.setPointerCapture(e.pointerId)
      start = controls.knobs[k.index]
      y0 = e.clientY
    })
    g.addEventListener('pointermove', (e) => {
      if (g.hasPointerCapture(e.pointerId)) controls.setKnob(k.index, start - (e.clientY - y0) / DRAG_PX)
    })
    g.addEventListener('dblclick', () => controls.setKnob(k.index, controls.defaults[k.index]))
    describe(g, () => knobText(k), readout)
    draws.push(() => {
      const a = ((controls.knobs[k.index] - 0.5) * SWEEP * Math.PI) / 180
      mark.setAttribute('x1', k.x + 12 * Math.sin(a))
      mark.setAttribute('y1', k.y - 12 * Math.cos(a))
      mark.setAttribute('x2', k.x + 30 * Math.sin(a))
      mark.setAttribute('y2', k.y - 30 * Math.cos(a))
      label.textContent = controls.shifted ? (SHORT_SECOND[k.index] ?? '—') : SHORT[k.index]
    })
  }

  for (const f of FADERS) {
    const g = svg('g', { class: 'control', tabindex: 0, 'aria-label': f.code }, board)
    svg('line', { class: 'slot', x1: f.x, y1: f.y0, x2: f.x, y2: f.y1 }, g)
    svg('rect', { x: f.x - 22, y: f.y0 - 20, width: 44, height: f.y1 - f.y0 + 40, fill: 'transparent' }, g)
    const cap = svg('rect', { class: 'cap', x: f.x - 16, width: 32, height: 10, rx: 3 }, g)
    svg('text', { class: 'code', x: f.x, y: f.y0 - 28 }, g).textContent = f.code
    const label = svg('text', { x: f.x, y: f.y1 + 34 }, g)
    const set = (e) => controls.setKnob(f.index, clamp((f.y1 - point(e).y) / (f.y1 - f.y0)))
    g.addEventListener('pointerdown', (e) => (g.setPointerCapture(e.pointerId), set(e)))
    g.addEventListener('pointermove', (e) => g.hasPointerCapture(e.pointerId) && set(e))
    g.addEventListener('dblclick', () => controls.setKnob(f.index, controls.defaults[f.index]))
    describe(g, () => knobText(f), readout)
    draws.push(() => {
      cap.setAttribute('y', f.y1 - controls.knobs[f.index] * (f.y1 - f.y0) - 5)
      label.textContent = controls.shifted ? SHORT_SECOND[f.index] : SHORT[f.index]
    })
  }

  for (const s of SWITCHES) {
    const g = svg('g', { class: 'control switch', tabindex: 0, 'aria-label': s.code }, board)
    svg('rect', { class: 'shape', x: s.x - 16, y: s.y - 34, width: 32, height: 68, rx: 10 }, g)
    const dots = [1, 0, 2].map((pos, i) => [pos, svg('circle', { class: 'pos', cx: s.x, cy: s.y - 20 + i * 20, r: 6 }, g)])
    svg('text', { class: 'code', x: s.x + 38 * s.side, y: s.y + 4 }, g).textContent = s.code
    g.addEventListener('pointerdown', (e) => {
      const p = controls.switches[s.index]
      const up = point(e).y < s.y
      controls.setSwitch(s.index, up ? (p === 2 ? 0 : 1) : p === 1 ? 0 : 2)
      readout.textContent = `${s.code} · ${SWITCH_HELP[s.index][controls.switches[s.index]]}`
    })
    describe(g, () => `${s.code} · ${SWITCH_HELP[s.index][controls.switches[s.index]]}`, readout)
    draws.push(() => dots.forEach(([pos, d]) => d.classList.toggle('on', pos === controls.switches[s.index])))
  }

  PADS.forEach((p, i) => {
    const code = `P${String(i).padStart(2, '0')}`
    const g = svg('g', { class: 'control pad', tabindex: 0, 'aria-label': code }, board)
    svg('circle', { class: 'shape', cx: p.x, cy: p.y, r: 38 }, g)
    const fill = svg('circle', { class: 'fill', cx: p.x, cy: p.y, r: 38 }, g)
    svg('text', { class: 'code', x: p.x, y: p.y + 4 }, g).textContent = code
    svg('text', { x: p.x, y: p.y + 56 }, g).textContent = padName(i)
    g.addEventListener('pointerdown', (e) => {
      g.setPointerCapture(e.pointerId)
      controls.press(i, `p${e.pointerId}`, e.pointerType === 'pen' ? e.pressure : null)
    })
    const up = (e) => controls.release(`p${e.pointerId}`)
    g.addEventListener('pointerup', up)
    g.addEventListener('pointercancel', up)
    describe(g, () => `${code} · ${padName(i)}`, readout)
    draws.push(() => {
      const on = controls.isHeld(i)
      g.classList.toggle('on', on)
      fill.setAttribute('opacity', on ? 0.15 + 0.5 * controls.pressure(i) : 0)
    })
  })

  const led = svg('circle', { class: 'led', cx: 300, cy: 70, r: 7, opacity: 0 }, board)
  svg('circle', { class: 'ledring', cx: 300, cy: 70, r: 10 }, board)

  return function draw(view) {
    for (const d of draws) d()
    led.setAttribute('opacity', view.led)
  }
}
