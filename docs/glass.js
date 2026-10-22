// The hourglass view: the recording in the upper bulb, its nine places as bands you
// press, and every grain falling through the neck while it plays. The controls sit
// around the glass, one per function.

import { KNOB, POS, PAD, PLACE_PADS, KNOB_HELP, SWITCH_HELP } from './controls.js'
import { html, dial, slider, segmented, holdButton } from './ui.js'

const HANDLE_PX = 14 // how near a selection edge counts as grabbing it

export function buildGlass(root, controls, readout) {
  const left = html('div', { class: 'column' }, root)
  const centre = html('div', { class: 'centre' }, root)
  const right = html('div', { class: 'column' }, root)
  const canvas = html('canvas', { 'aria-label': 'Hourglass: the recording, its places and the grains' }, centre)
  const below = html('div', { class: 'below' }, root)
  const draws = []

  // One dial per function. Main layer: the knob itself. Second layer: its value.
  const main = (label, k) =>
    draws.push(dial(left, {
      label, readout,
      get: () => controls.knobs[k],
      set: (v) => controls.setKnob(k, v),
      reset: () => controls.setKnob(k, controls.defaults[k]),
      help: () => KNOB_HELP[k][0],
    }))
  const second = (parent, label, k) =>
    draws.push(dial(parent, {
      label, readout,
      get: () => controls.second[k],
      set: (v) => controls.setSecond(k, v),
      reset: () => controls.setSecond(k, controls.secondDefaults[k]),
      help: () => KNOB_HELP[k][1],
    }))
  html('h3', {}, left).textContent = 'Grain'
  main('Size', KNOB.S31)
  second(left, 'Window', KNOB.S31)
  main('Pitch', KNOB.S33)
  main('Random pitch', KNOB.S34)
  main('Scatter', KNOB.S32)
  html('h3', {}, right).textContent = 'Space'
  second(right, 'Spread', KNOB.S32)
  second(right, 'Tone', KNOB.S35)
  second(right, 'Reverb', KNOB.S34)
  second(right, 'Dry / wet', KNOB.S36)
  second(right, 'Resample', KNOB.S37)

  // Below the glass: time and sand
  const time = html('div', { class: 'group' }, below)
  draws.push(segmented(time, {
    label: 'Playback', readout,
    options: [[POS.up, 'Forward'], [POS.centre, 'Freeze'], [POS.down, 'Reverse']],
    get: () => controls.switches[0],
    set: (p) => controls.setSwitch(0, p),
    help: (p) => SWITCH_HELP[0][p],
  }))
  draws.push(slider(time, {
    label: () => (controls.switches[0] === POS.centre ? 'Scrub' : 'Speed'), readout, centre: true,
    get: () => controls.knobs[KNOB.S36],
    set: (v) => controls.setKnob(KNOB.S36, v),
    reset: () => controls.setKnob(KNOB.S36, controls.defaults[KNOB.S36]),
    help: () => KNOB_HELP[KNOB.S36][0],
  }))
  const sand = html('div', { class: 'group' }, below)
  draws.push(segmented(sand, {
    label: 'Starts', readout,
    options: [[POS.up, 'Periodic'], [POS.centre, 'Half'], [POS.down, 'Random']],
    get: () => controls.switches[1],
    set: (p) => controls.setSwitch(1, p),
    help: (p) => SWITCH_HELP[1][p],
  }))
  draws.push(slider(sand, {
    label: 'Flow', readout,
    get: () => controls.knobs[KNOB.S37],
    set: (v) => controls.setKnob(KNOB.S37, v),
    reset: () => controls.setKnob(KNOB.S37, controls.defaults[KNOB.S37]),
    help: () => KNOB_HELP[KNOB.S37][0],
  }))
  const hands = html('div', { class: 'group hands' }, below)
  draws.push(holdButton(hands, {
    label: 'Record', readout,
    press: (who) => controls.press(PAD.record, who),
    release: (who) => controls.release(who),
    isOn: () => controls.isHeld(PAD.record),
    help: 'tap: keep the last 10 s. Hold: keep the touch. (Space)',
  }))
  draws.push(holdButton(hands, {
    label: 'Stop', readout,
    press: (who) => controls.press(PAD.stop, who),
    release: (who) => controls.release(who),
    isOn: () => controls.isHeld(PAD.stop),
    help: 'hold: no new grains; those playing ring out. (0)',
  }))

  // The glass: drag the selection's edges; press the places
  let geometry = null
  let dragging = null // 'start' | 'end'
  canvas.addEventListener('pointerdown', (e) => {
    if (!geometry) return
    const { x, y } = local(e)
    const g = geometry
    if (y >= g.band.y0 && y <= g.band.y1) {
      const place = Math.min(8, Math.max(0, Math.floor(((x - g.left) / (g.right - g.left)) * 9)))
      canvas.setPointerCapture(e.pointerId)
      controls.press(PLACE_PADS[place], `p${e.pointerId}`, e.pointerType === 'pen' ? e.pressure : null)
      readout.textContent = `Place ${place + 1}: hold to pour grains from this ninth of the selection; longer pours more`
      return
    }
    if (y <= g.wave.y1 + 8) {
      canvas.setPointerCapture(e.pointerId)
      const pos = g.toPos(x)
      const s = controls.knobs[KNOB.S30]
      const t = controls.knobs[KNOB.S35]
      dragging = Math.abs(pos - s) < Math.abs(pos - t) ? KNOB.S30 : KNOB.S35
      controls.setKnob(dragging, pos)
    }
  })
  canvas.addEventListener('pointermove', (e) => {
    if (dragging !== null && canvas.hasPointerCapture(e.pointerId)) controls.setKnob(dragging, geometry.toPos(local(e).x))
    else if (!canvas.hasPointerCapture(e.pointerId) && geometry) hoverGlass(local(e))
  })
  const up = (e) => {
    dragging = null
    controls.release(`p${e.pointerId}`)
  }
  canvas.addEventListener('pointerup', up)
  canvas.addEventListener('pointercancel', up)
  canvas.addEventListener('pointerleave', () => !dragging && (readout.textContent = ' '))

  function hoverGlass({ x, y }) {
    const g = geometry
    canvas.style.cursor = y <= g.wave.y1 + 8 ? 'ew-resize' : y >= g.band.y0 && y <= g.band.y1 ? 'pointer' : 'default'
    if (y <= g.wave.y1 + 8) {
      const near = [KNOB.S30, KNOB.S35].some((k) => Math.abs(g.toX(controls.knobs[k]) - x) < HANDLE_PX)
      readout.textContent = `The recording. Drag ${near ? 'this edge' : 'an edge'} to set the selection.`
    } else if (y >= g.band.y0 && y <= g.band.y1) readout.textContent = 'The nine places: press and hold one to pour grains from it'
    else readout.textContent = 'The grains: each falls through the neck while it plays'
  }

  function local(e) {
    const r = canvas.getBoundingClientRect()
    return { x: e.clientX - r.left, y: e.clientY - r.top }
  }

  return function draw(view, colours) {
    for (const d of draws) d()
    geometry = drawGlass(canvas, view, controls, colours)
  }
}

function drawGlass(canvas, view, controls, c) {
  const dpr = devicePixelRatio || 1
  const w = canvas.clientWidth
  const h = canvas.clientHeight
  if (canvas.width !== Math.round(w * dpr) || canvas.height !== Math.round(h * dpr)) {
    canvas.width = Math.round(w * dpr)
    canvas.height = Math.round(h * dpr)
  }
  const g = canvas.getContext('2d')
  g.setTransform(dpr, 0, 0, dpr, 0, 0)
  g.clearRect(0, 0, w, h)

  // The glass: no wider than its height allows, straight walls rounding into the neck
  const gw = Math.min(w, h * 1.1)
  const m = (w - gw) / 2 + 10
  const cx = w / 2
  const neckY = h * 0.52
  const neck = 5
  const left = m + 14
  const right = w - m - 14
  const top = 10
  const bottom = h - 10
  const shoulder = (neckY - top) * 0.55 // where the walls start to round
  g.strokeStyle = c.line
  g.lineWidth = 1.5
  g.beginPath()
  g.moveTo(m, top)
  g.lineTo(w - m, top)
  g.lineTo(w - m, top + shoulder * 0.5)
  g.bezierCurveTo(w - m, top + shoulder * 1.4, cx + neck, neckY - (neckY - top) * 0.2, cx + neck, neckY)
  g.bezierCurveTo(cx + neck, neckY + (bottom - neckY) * 0.2, w - m, bottom - shoulder * 1.4, w - m, bottom - shoulder * 0.5)
  g.lineTo(w - m, bottom)
  g.lineTo(m, bottom)
  g.lineTo(m, bottom - shoulder * 0.5)
  g.bezierCurveTo(m, bottom - shoulder * 1.4, cx - neck, neckY + (bottom - neckY) * 0.2, cx - neck, neckY)
  g.bezierCurveTo(cx - neck, neckY - (neckY - top) * 0.2, m, top + shoulder * 1.4, m, top + shoulder * 0.5)
  g.closePath()
  g.stroke()

  const toX = (pos) => left + pos * (right - left)
  const toPos = (x) => Math.min(1, Math.max(0, (x - left) / (right - left)))
  const wave = { y0: top + 14, y1: top + 14 + Math.min(70, h * 0.13) }
  const band = { y0: wave.y1 + 14, y1: wave.y1 + 14 + 26 }
  const mid = (wave.y0 + wave.y1) / 2

  // The recording; the selection darker
  const s0 = view.start
  const s1 = view.start + view.length
  const outline = view.outline
  const cols = outline.length
  if (cols) {
    const colW = (right - left) / cols
    for (let i = 0; i < cols; i++) {
      const pos = (i + 0.5) / cols
      const amp = Math.min(1, outline[i] * 1.5) * ((wave.y1 - wave.y0) / 2)
      g.fillStyle = pos >= s0 && pos <= s1 ? c.fg : c.line
      g.fillRect(left + i * colW, mid - amp, Math.max(1, colW - 0.5), 2 * amp + 0.5)
    }
  } else {
    g.fillStyle = c.muted
    g.font = '13px -apple-system, sans-serif'
    g.textAlign = 'center'
    g.fillText(view.running ? 'Nothing recorded yet: press Record' : 'Press Start', cx, mid + 4)
  }

  // Selection edges
  g.strokeStyle = c.fg
  g.lineWidth = 1.5
  for (const k of [KNOB.S30, KNOB.S35]) {
    const x = toX(controls.knobs[k])
    g.beginPath()
    g.moveTo(x, wave.y0 - 6)
    g.lineTo(x, wave.y1 + 6)
    g.stroke()
  }

  // The nine places: held ones lit, with their part of the recording
  const cellW = (right - left) / 9
  for (let p = 0; p < 9; p++) {
    const pad = PLACE_PADS[p]
    const on = controls.isHeld(pad)
    g.strokeStyle = on ? c.accent : c.line
    g.lineWidth = 1
    g.strokeRect(left + p * cellW + 2, band.y0, cellW - 4, band.y1 - band.y0)
    if (on) {
      g.globalAlpha = 0.15 + 0.5 * controls.pressure(pad)
      g.fillStyle = c.accent
      g.fillRect(left + p * cellW + 2, band.y0, cellW - 4, band.y1 - band.y0)
      g.fillRect(toX(s0 + (view.length * p) / 9), wave.y0, ((right - left) * view.length) / 9, wave.y1 - wave.y0)
      g.globalAlpha = 1
    }
    g.fillStyle = c.muted
    g.font = '11px -apple-system, sans-serif'
    g.textAlign = 'center'
    g.fillText(String(p + 1), left + (p + 0.5) * cellW, band.y1 - 9)
  }

  // Where each source is reading: the playhead, and the places pouring
  if (cols) {
    for (let s = 0; s < 10; s++) {
      if (!view.sources[s * 2 + 1]) continue
      const span = s === 0 ? [s0, view.length] : [s0 + (view.length * (s - 1)) / 9, view.length / 9]
      const x = toX(span[0] + view.sources[s * 2] * span[1])
      g.strokeStyle = s === 0 ? c.fg : c.accent
      g.lineWidth = s === 0 ? 1 : 2
      g.beginPath()
      g.moveTo(x, wave.y0 - 6)
      g.lineTo(x, wave.y1 + 6)
      g.stroke()
    }
  }

  // The grains: from where each reads, through the neck, to the bottom by its pan
  const from = band.y1 + 8
  const base = bottom - 10
  const spreadW = (right - left) * 0.42
  g.fillStyle = c.fg
  const grains = view.grains
  for (let i = 0; i < grains.length; i += 3) {
    const p = grains[i + 1]
    let x, y
    if (p < 0.5) {
      const t = p / 0.5
      x = toX(grains[i]) + (cx - toX(grains[i])) * t * t
      y = from + (neckY - from) * t
    } else {
      const t = (p - 0.5) / 0.5
      x = cx + (grains[i + 2] - 0.5) * 2 * spreadW * Math.sqrt(t)
      y = neckY + (base - neckY) * t
    }
    g.globalAlpha = 0.3 + 0.7 * Math.sin(Math.PI * p)
    g.beginPath()
    g.arc(x, y, 2.2, 0, 2 * Math.PI)
    g.fill()
  }
  g.globalAlpha = 1

  return { left, right, wave, band, toX, toPos }
}
