// The hourglass view: the recording in the upper bulb, its places as bands you
// press, and every grain falling through the neck while it plays. The controls sit
// around the glass, one per function.

import { KNOB, POS, PAD, PLACE_PADS, KNOB_HELP, SWITCH_HELP } from './controls.js'
import { html, dial, slider, segmented, holdButton } from './ui.js'

const HANDLE_PX = 14 // how near a selection edge counts as grabbing it
const PLACES = PLACE_PADS.length

export function buildGlass(root, controls, readout) {
  const left = html('div', { class: 'column' }, root)
  const centre = html('div', { class: 'centre' }, root)
  const right = html('div', { class: 'column' }, root)
  const canvas = html('canvas', { 'aria-label': 'Hourglass: the recording, its places and the grains' }, centre)
  const below = html('div', { class: 'below' }, root)
  const draws = []

  // One dial per function. Main layer: the knob itself. Second layer: its value.
  const main = (parent, label, k) =>
    draws.push(dial(parent, {
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
  // Groups as on the instrument: each grain, its pitch, the space around it
  const group = (parent, title) => {
    html('div', { class: 'label' }, parent).textContent = title
    return html('div', { class: 'dials' }, parent)
  }
  const grain = group(left, 'Grain')
  main(grain, 'Size', KNOB.S31)
  second(grain, 'Variation', KNOB.S32)
  second(grain, 'Shape', KNOB.S31)
  main(grain, 'Random', KNOB.S32)
  second(grain, 'Reverse', KNOB.S30)
  const pitch = group(right, 'Pitch')
  main(pitch, 'Pitch', KNOB.S33)
  main(pitch, 'Random', KNOB.S34)
  second(pitch, 'Harmonic', KNOB.S34)
  const space = group(right, 'Space')
  second(space, 'Tone', KNOB.S35)
  second(space, 'Reverb', KNOB.S33)
  second(space, 'Dry / wet', KNOB.S36)

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
    label: 'Density', readout,
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
    help: 'tap: keep the last 8 s. Hold: keep the touch. (Space)',
  }))
  second(hands, 'Resample', KNOB.S37)
  draws.push(holdButton(hands, {
    label: 'Stop', readout,
    press: (who) => controls.press(PAD.stop, who),
    release: (who) => controls.release(who),
    isOn: () => controls.isHeld(PAD.stop),
    help: 'hold: no new grains; those playing ring out. (0)',
  }))
  draws.push(holdButton(hands, {
    label: 'Reset', readout,
    press: () => controls.reset(),
    release: () => {},
    isOn: () => false,
    help: 'the sound back to the defaults: the recording played as it is. (R)',
  }))

  // The glass: drag the selection's edges; press the places
  let geometry = null
  let dragging = null // 'start' | 'end'
  canvas.addEventListener('pointerdown', (e) => {
    if (!geometry) return
    const { x, y } = local(e)
    const g = geometry
    if (y >= g.band.y0 && y <= g.band.y1) {
      const place = Math.min(PLACES - 1, Math.max(0, Math.floor(((x - g.left) / (g.right - g.left)) * PLACES)))
      canvas.setPointerCapture(e.pointerId)
      controls.press(PLACE_PADS[place], `p${e.pointerId}`, e.pointerType === 'pen' ? e.pressure : null)
      readout.textContent = `Place ${place + 1}: hold to pour grains from this part of the selection; longer pours more`
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
    } else if (y >= g.band.y0 && y <= g.band.y1) readout.textContent = 'The places: press and hold one to pour grains from it'
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

// The glass's outline, as a path: straight walls rounding into the neck
function glassPath(g, m, w, top, bottom, cx, neckY, neck) {
  const shoulder = (neckY - top) * 0.55
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
}

const PILE_BINS = 72
const PILE_FADE_S = 6 // the pile halves in about this long
const sand = { pile: new Float32Array(PILE_BINS), last: 0 }

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
  const now = performance.now()
  const dt = Math.min(0.1, (now - (sand.last || now)) / 1000)
  sand.last = now

  const gw = Math.min(w, h * 1.1)
  const m = (w - gw) / 2 + 10
  const cx = w / 2
  const neckY = h * 0.52
  const neck = 5
  const left = m + 16
  const right = w - m - 16
  const top = 10
  const bottom = h - 10

  // The glass: a faint body, a soft highlight down its left side, the outline
  glassPath(g, m, w, top, bottom, cx, neckY, neck)
  const body = g.createLinearGradient(m, 0, w - m, 0)
  body.addColorStop(0, c.glassEdge)
  body.addColorStop(0.5, c.glass)
  body.addColorStop(1, c.glassEdge)
  g.fillStyle = body
  g.fill()
  g.save()
  g.clip()
  const shine = g.createLinearGradient(m, 0, m + gw * 0.18, 0)
  shine.addColorStop(0, c.shine)
  shine.addColorStop(1, 'transparent')
  g.fillStyle = shine
  g.fillRect(m, top, gw * 0.18, bottom - top)

  const toX = (pos) => left + pos * (right - left)
  const toPos = (x) => Math.min(1, Math.max(0, (x - left) / (right - left)))
  const wave = { y0: top + 16, y1: top + 16 + Math.min(72, h * 0.13) }
  const band = { y0: wave.y1 + 12, y1: wave.y1 + 12 + 24 }
  const mid = (wave.y0 + wave.y1) / 2
  const s0 = view.start
  const s1 = view.start + view.length

  // The recording as sand: the selection gold, the rest faint
  const outline = view.outline
  const cols = outline.length
  if (cols) {
    const colW = (right - left) / cols
    for (let i = 0; i < cols; i++) {
      const pos = (i + 0.5) / cols
      const amp = Math.min(1, outline[i] * 1.6) * ((wave.y1 - wave.y0) / 2)
      g.fillStyle = pos >= s0 && pos <= s1 ? c.sand : c.line
      g.fillRect(left + i * colW, mid - amp, Math.max(1, colW - 0.6), 2 * amp + 0.6)
    }
    // Outside the selection, veiled
    g.fillStyle = c.veil
    g.fillRect(left, wave.y0 - 4, toX(s0) - left, wave.y1 - wave.y0 + 8)
    g.fillRect(toX(s1), wave.y0 - 4, right - toX(s1), wave.y1 - wave.y0 + 8)
  } else {
    g.fillStyle = c.dim
    g.font = `500 11px ${c.mono}`
    g.textAlign = 'center'
    g.fillText(view.running ? 'NOTHING RECORDED YET · PRESS RECORD' : 'PRESS START', cx, mid + 4)
  }

  // Selection edges, with grips
  g.strokeStyle = c.ink
  g.fillStyle = c.ink
  g.lineWidth = 1.5
  for (const k of [KNOB.S30, KNOB.S35]) {
    const x = toX(controls.knobs[k])
    g.beginPath()
    g.moveTo(x, wave.y0 - 8)
    g.lineTo(x, wave.y1 + 8)
    g.stroke()
    g.beginPath()
    g.arc(x, wave.y0 - 8, 3, 0, 2 * Math.PI)
    g.fill()
  }

  // The places: a row of pills; a held one lit, with its part of the recording
  const cellW = (right - left) / PLACES
  for (let p = 0; p < PLACES; p++) {
    const pad = PLACE_PADS[p]
    const on = controls.isHeld(pad)
    const x = left + p * cellW + 3
    const r = (band.y1 - band.y0) / 2
    g.beginPath()
    g.roundRect(x, band.y0, cellW - 6, band.y1 - band.y0, r)
    if (on) {
      const k = controls.pressure(pad)
      g.fillStyle = c.accent
      g.globalAlpha = 0.25 + 0.6 * k
      g.fill()
      g.globalAlpha = 0.12 + 0.25 * k
      g.fillRect(toX(s0 + (view.length * p) / PLACES), wave.y0, ((right - left) * view.length) / PLACES, wave.y1 - wave.y0)
      g.globalAlpha = 1
    } else {
      g.strokeStyle = c.line
      g.lineWidth = 1
      g.stroke()
    }
    g.fillStyle = on ? c.ink : c.dim
    g.font = `500 10px ${c.mono}`
    g.textAlign = 'center'
    g.fillText(String(p + 1), x + (cellW - 6) / 2, band.y1 - 8)
  }

  // Where each source reads: the playhead, and the places pouring
  if (cols) {
    for (let s = 0; s <= PLACES; s++) {
      if (!view.sources[s * 2 + 1]) continue
      const span = s === 0 ? [s0, view.length] : [s0 + (view.length * (s - 1)) / PLACES, view.length / PLACES]
      const x = toX(span[0] + view.sources[s * 2] * span[1])
      g.strokeStyle = s === 0 ? c.ink : c.accent
      g.lineWidth = s === 0 ? 1 : 2
      g.beginPath()
      g.moveTo(x, wave.y0 - 4)
      g.lineTo(x, wave.y1 + 4)
      g.stroke()
    }
  }

  // The grains. Forward: from where each reads, down through the neck, landing by its
  // pan. Reverse: the glass flipped, they rise from the pile. Frozen: on its side,
  // they hover where they read.
  const mode = controls.switches[0] // 1 forward, 0 freeze, 2 reverse
  const from = band.y1 + 10
  const pileTop = bottom - 6
  const spreadW = (right - left) * 0.44
  const at = (pos, p, pan) => {
    if (mode === 0) {
      const x0 = toX(pos)
      return [x0 + (pan - 0.5) * 14 * Math.sin(Math.PI * p), from + 18 + 26 * Math.sin(Math.PI * p) * (0.5 + pan)]
    }
    const t = mode === 2 ? 1 - p : p
    if (t < 0.5) {
      const u = t / 0.5
      return [toX(pos) + (cx - toX(pos)) * u * u, from + (neckY - from) * u]
    }
    const u = (t - 0.5) / 0.5
    return [cx + (pan - 0.5) * 2 * spreadW * Math.sqrt(u), neckY + (pileTop - neckY) * u]
  }
  const grains = view.grains
  const count = grains.length / 3
  // The stream through the neck, as dense as the grains
  if (mode !== 0 && count) {
    const stream = g.createLinearGradient(0, neckY - 40, 0, neckY + 40)
    stream.addColorStop(0, 'transparent')
    stream.addColorStop(0.5, c.sand)
    stream.addColorStop(1, 'transparent')
    g.fillStyle = stream
    g.globalAlpha = Math.min(0.6, count / 40)
    g.fillRect(cx - 1.5, neckY - 40, 3, 80)
    g.globalAlpha = 1
  }
  g.globalCompositeOperation = c.dark ? 'lighter' : 'source-over'
  for (let i = 0; i < grains.length; i += 3) {
    const pos = grains[i], p = grains[i + 1], pan = grains[i + 2]
    const window = Math.sin(Math.PI * p)
    const [x, y] = at(pos, p, pan)
    const [tx, ty] = at(pos, Math.max(0, p - 0.06), pan)
    g.strokeStyle = c.sand
    g.globalAlpha = 0.25 * window
    g.lineWidth = 1.5
    g.beginPath()
    g.moveTo(tx, ty)
    g.lineTo(x, y)
    g.stroke()
    const r = 1.4 + 2.2 * window
    const glow = g.createRadialGradient(x, y, 0, x, y, r * 3)
    glow.addColorStop(0, c.sand)
    glow.addColorStop(1, 'transparent')
    g.globalAlpha = 0.35 * window
    g.fillStyle = glow
    g.fillRect(x - r * 3, y - r * 3, r * 6, r * 6)
    g.globalAlpha = 0.5 + 0.5 * window
    g.fillStyle = c.sand
    g.beginPath()
    g.arc(x, y, r, 0, 2 * Math.PI)
    g.fill()
    // Landing: the grain adds to the pile where it falls
    if (mode === 1 && p > 0.85) {
      const bin = Math.max(0, Math.min(PILE_BINS - 1, Math.floor(((x - left) / (right - left)) * PILE_BINS)))
      sand.pile[bin] += dt * 1.2
    }
  }
  g.globalCompositeOperation = 'source-over'
  g.globalAlpha = 1

  // The pile: a soft dune that fades unless fed
  const fade = Math.pow(0.5, dt / PILE_FADE_S)
  const heights = Array.from(sand.pile, (v, i) => {
    sand.pile[i] = v * fade
    const l = sand.pile[Math.max(0, i - 1)], r = sand.pile[Math.min(PILE_BINS - 1, i + 1)]
    return (l + 2 * v + r) / 4
  })
  const maxH = (bottom - neckY) * 0.35
  g.beginPath()
  g.moveTo(m, bottom)
  heights.forEach((v, i) => {
    const x = left + ((i + 0.5) / PILE_BINS) * (right - left)
    g.lineTo(x, bottom - Math.min(maxH, v * maxH))
  })
  g.lineTo(w - m, bottom)
  g.closePath()
  const dune = g.createLinearGradient(0, bottom - maxH, 0, bottom)
  dune.addColorStop(0, c.sand)
  dune.addColorStop(1, c.sandDeep)
  g.fillStyle = dune
  g.globalAlpha = 0.85
  g.fill()
  g.globalAlpha = 1
  g.restore()

  // The outline over everything
  glassPath(g, m, w, top, bottom, cx, neckY, neck)
  g.strokeStyle = c.dim
  g.lineWidth = 1.5
  g.stroke()

  return { left, right, wave, band, toX, toPos }
}
