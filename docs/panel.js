// The panel view: the Simple Touch with a Clessidra faceplate, drawn to the official
// template (faceplate.js). The overlay carries each knob's functions, as a printed
// faceplate would: the main one above, the second layer (P11) below. The pads are
// the stones of the lower half. Plays like the board.

import { PAD, PLACE_PADS, KNOB_HELP, SWITCH_HELP, clamp } from './controls.js'
import { svg, describe } from './ui.js'
import * as F from './faceplate.js'

const DRAG_PX = 200
const SWEEP = 270
const CAP_R = 13.5
const STONE_GAP = 4.5

const MAIN = ['Start', 'Size', 'Scatter', 'Pitch', 'Random', 'End', 'Speed', 'Flow']
const SECOND = [null, 'Window', 'Spread', null, 'Reverb', 'Tone', 'Dry/wet', 'Resample']
const SWITCH_TEXT = [{ 1: 'FWD', 0: 'FRZ', 2: 'REV' }, { 1: 'PER', 0: 'HALF', 2: 'RND' }]

function padName(i) {
  if (i === PAD.stop) return 'Stop'
  if (i === PAD.record) return 'Record'
  if (i === PAD.shift) return 'Layer'
  return String(PLACE_PADS.indexOf(i) + 1)
}

// The stones: each place pad's Voronoi cell within the lower half
function cells(sites, box) {
  const clip = (poly, a, b) => {
    // Keep the side of the bisector of a and b nearer a
    const mx = (a[0] + b[0]) / 2, my = (a[1] + b[1]) / 2
    const nx = b[0] - a[0], ny = b[1] - a[1]
    const side = (p) => (p[0] - mx) * nx + (p[1] - my) * ny
    const out = []
    poly.forEach((p, i) => {
      const q = poly[(i + 1) % poly.length]
      const sp = side(p), sq = side(q)
      if (sp <= 0) out.push(p)
      if (sp * sq < 0) {
        const t = sp / (sp - sq)
        out.push([p[0] + t * (q[0] - p[0]), p[1] + t * (q[1] - p[1])])
      }
    })
    return out
  }
  return sites.map((a) => sites.reduce((poly, b) => (b === a ? poly : clip(poly, a, b)), box))
}

// Corner cutting (Chaikin): straight cells become pebbles
function smooth(poly, rounds = 3) {
  for (let r = 0; r < rounds; r++) {
    poly = poly.flatMap((p, i) => {
      const q = poly[(i + 1) % poly.length]
      return [
        [0.75 * p[0] + 0.25 * q[0], 0.75 * p[1] + 0.25 * q[1]],
        [0.25 * p[0] + 0.75 * q[0], 0.25 * p[1] + 0.75 * q[1]],
      ]
    })
  }
  return poly
}

const pathOf = (poly) => `M${poly.map((p) => `${p[0].toFixed(1)} ${p[1].toFixed(1)}`).join(' L')} Z`

export function buildPanel(board, controls, readout) {
  const B = F.BOARD
  const pad = 8
  board.setAttribute('viewBox', `${B.x - pad} ${B.y - pad} ${B.width + 2 * pad} ${B.height + 2 * pad}`)
  board.innerHTML = ''
  const draws = []
  const point = (e) => {
    const pt = board.createSVGPoint()
    pt.x = e.clientX
    pt.y = e.clientY
    return pt.matrixTransform(board.getScreenCTM().inverse())
  }

  // Materials
  const defs = svg('defs', {}, board)
  defs.innerHTML = `
    <radialGradient id="pcb" cx="50%" cy="40%" r="75%"><stop offset="0" stop-color="#1b1b1c"/><stop offset="1" stop-color="#0c0c0d"/></radialGradient>
    <linearGradient id="plate" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#202022"/><stop offset="1" stop-color="#151517"/></linearGradient>
    <linearGradient id="gold" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#d9bd85"/><stop offset="0.55" stop-color="#b8975c"/><stop offset="1" stop-color="#8d7041"/></linearGradient>
    ${[-70, -48, -25, 12, 35, 58].map((a, i) => `<pattern id="grain${i}" width="3.2" height="3.2" patternUnits="userSpaceOnUse" patternTransform="rotate(${a})"><line x1="0" y1="0" x2="0" y2="3.2" stroke="#6e5530" stroke-width="0.55" opacity="0.55"/></pattern>`).join('')}
    <radialGradient id="cap" cx="40%" cy="35%" r="70%"><stop offset="0" stop-color="#3b3b3e"/><stop offset="0.6" stop-color="#161618"/><stop offset="1" stop-color="#050506"/></radialGradient>
    <linearGradient id="metal" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#eeeeee"/><stop offset="0.5" stop-color="#9c9c9c"/><stop offset="1" stop-color="#d4d4d4"/></linearGradient>
    <filter id="glow" x="-50%" y="-50%" width="200%" height="200%"><feGaussianBlur stdDeviation="4"/></filter>
    <clipPath id="stones-clip"><rect x="${B.x}" y="${B.y}" width="${B.width}" height="${B.height}" rx="9"/></clipPath>`

  // The board
  svg('rect', { x: B.x, y: B.y, width: B.width, height: B.height, rx: 9, fill: 'url(#pcb)' }, board)

  // The Daisy Seed under the cut-out: its two pin rows and its LED
  for (const y of [-71, 12]) {
    for (let i = 0; i < 20; i++) svg('circle', { cx: 100 + i * 10.2, cy: y, r: 2.2, fill: '#b8975c', opacity: 0.85 }, board)
  }
  svg('rect', { x: 150, y: -50, width: 70, height: 46, rx: 3, fill: '#0a0a0b', stroke: '#2a2a2c', 'stroke-width': 0.8 }, board)
  const ledGlow = svg('circle', { cx: 268, cy: -30, r: 9, fill: '#ff5a1f', filter: 'url(#glow)', opacity: 0 }, board)
  const led = svg('circle', { cx: 268, cy: -30, r: 2.6, fill: '#ff5a1f', opacity: 0.15 }, board)

  // The stones: places and stop, gold with an engraved grain
  const stoneG = svg('g', { 'clip-path': 'url(#stones-clip)' }, board)
  const lower = F.PADS.slice(0, 10)
  const box = [[B.x, F.OVERLAY_BOTTOM + 6], [B.x + B.width, F.OVERLAY_BOTTOM + 6], [B.x + B.width, B.y + B.height], [B.x, B.y + B.height]]
  const stones = cells(lower, box).map((poly) => smooth(poly))
  const padHits = []
  stones.forEach((poly, i) => {
    const d = pathOf(poly)
    const g = svg('g', { class: 'control stone', tabindex: 0, 'aria-label': `P${String(i).padStart(2, '0')}` }, stoneG)
    svg('path', { d, fill: 'url(#gold)' }, g)
    svg('path', { d, fill: `url(#grain${(i * 5) % 6})` }, g)
    const lit = svg('path', { d, fill: '#ff5a1f', opacity: 0 }, g)
    svg('path', { d, fill: 'none', stroke: '#0c0c0d', 'stroke-width': STONE_GAP * 2, 'stroke-linejoin': 'round' }, g)
    const [cx, cy] = F.PADS[i]
    svg('text', { class: 'engraved', x: cx, y: cy + 4 }, g).textContent = padName(i)
    padHits.push([i, g, lit])
  })

  // P10 and P11: two stones along the top edge
  for (const i of [PAD.record, PAD.shift]) {
    const [cx, cy] = F.PADS[i]
    const g = svg('g', { class: 'control stone', tabindex: 0, 'aria-label': `P${i}` }, board)
    const d = `M${cx - 30} ${cy} q0 -18 30 -18 q30 0 30 18 q0 18 -30 18 q-30 0 -30 -18 Z`
    svg('path', { d, fill: 'url(#gold)' }, g)
    svg('path', { d, fill: 'url(#grain2)' }, g)
    const lit = svg('path', { d, fill: '#ff5a1f', opacity: 0 }, g)
    svg('text', { class: 'engraved', x: cx, y: cy + 4 }, g).textContent = padName(i)
    padHits.push([i, g, lit])
  }

  for (const [i, g, lit] of padHits) {
    g.addEventListener('pointerdown', (e) => {
      g.setPointerCapture(e.pointerId)
      controls.press(i, `p${e.pointerId}`, e.pointerType === 'pen' ? e.pressure : null)
    })
    g.addEventListener('pointermove', (e) => e.pointerType === 'pen' && controls.pen.has(i) && controls.pen.set(i, e.pressure))
    const up = (e) => controls.release(`p${e.pointerId}`)
    g.addEventListener('pointerup', up)
    g.addEventListener('pointercancel', up)
    describe(g, () => `P${String(i).padStart(2, '0')} · ${i === PAD.shift ? 'Hold: second layer' : i === PAD.record ? 'Tap: keep the last 10 s. Hold: record.' : i === PAD.stop ? 'Hold: stop the flow' : `Place ${padName(i)}: hold to pour grains`}`, readout)
    draws.push(() => lit.setAttribute('opacity', controls.isHeld(i) ? 0.25 + 0.5 * controls.pressure(i) : 0))
  }

  // The overlay: the faceplate itself
  svg('path', { d: F.OVERLAY, fill: 'url(#plate)', stroke: '#3a3226', 'stroke-width': 0.8 }, board)
  svg('text', { class: 'title', x: 150.3, y: 141 }, board).textContent = 'Clessidra'
  const glyph = svg('g', { transform: 'translate(150.3 116)', class: 'glyph' }, board)
  svg('path', { d: 'M-6 -8 H6 L0 0 L6 8 H-6 L0 0 Z' }, glyph)

  for (const [x, y] of F.SCREWS) {
    svg('circle', { cx: x, cy: y, r: 4.6, fill: 'url(#metal)' }, board)
    svg('path', { d: `M${x - 2.6} ${y} H${x + 2.6} M${x} ${y - 2.6} V${y + 2.6}`, stroke: '#555', 'stroke-width': 0.8 }, board)
  }
  for (const j of F.JACKS) {
    svg('circle', { cx: j.x, cy: j.y, r: 8.5, fill: 'url(#metal)' }, board)
    svg('circle', { cx: j.x, cy: j.y, r: 4.6, fill: '#050505' }, board)
    svg('text', { class: 'print', x: j.x, y: j.y + 18 }, board).textContent = j.name
  }

  // Knobs: scale ticks printed around each hole, the cap and its line, both functions
  const labels = []
  F.KNOBS.forEach(([x, y], index) => {
    for (let t = 0; t <= 10; t++) {
      const a = ((t / 10 - 0.5) * SWEEP * Math.PI) / 180
      const r0 = F.KNOB_R + 2.5, r1 = F.KNOB_R + (t % 5 === 0 ? 6 : 4)
      svg('line', { class: 'tick', x1: x + r0 * Math.sin(a), y1: y - r0 * Math.cos(a), x2: x + r1 * Math.sin(a), y2: y - r1 * Math.cos(a) }, board)
    }
    const g = svg('g', { class: 'control', tabindex: 0, 'aria-label': `S3${index}` }, board)
    svg('circle', { cx: x, cy: y, r: F.KNOB_R, fill: '#08080a' }, g)
    svg('circle', { cx: x, cy: y, r: CAP_R, fill: 'url(#cap)', stroke: '#2c2c30', 'stroke-width': 0.8 }, g)
    const mark = svg('line', { class: 'pointer' }, g)
    const main = svg('text', { class: 'print', x, y: y - F.KNOB_R - 9 }, board)
    const second = svg('text', { class: 'print second', x, y: y + F.KNOB_R + 13 }, board)
    main.textContent = MAIN[index]
    second.textContent = SECOND[index] ?? ''
    labels.push([main, second])
    let start = 0
    let y0 = 0
    g.addEventListener('pointerdown', (e) => {
      g.setPointerCapture(e.pointerId)
      start = controls.knobs[index]
      y0 = e.clientY
    })
    g.addEventListener('pointermove', (e) => g.hasPointerCapture(e.pointerId) && controls.setKnob(index, start - (e.clientY - y0) / DRAG_PX))
    g.addEventListener('dblclick', () => controls.setKnob(index, controls.defaults[index]))
    g.addEventListener('wheel', (e) => (e.preventDefault(), controls.setKnob(index, controls.knobs[index] - Math.sign(e.deltaY) * 0.01)), { passive: false })
    describe(g, () => `S3${index} · ${controls.shifted ? (KNOB_HELP[index][1] ?? 'no second layer') : KNOB_HELP[index][0]}`, readout)
    draws.push(() => {
      const a = ((controls.knobs[index] - 0.5) * SWEEP * Math.PI) / 180
      mark.setAttribute('x1', x + 4 * Math.sin(a))
      mark.setAttribute('y1', y - 4 * Math.cos(a))
      mark.setAttribute('x2', x + (CAP_R - 2) * Math.sin(a))
      mark.setAttribute('y2', y - (CAP_R - 2) * Math.cos(a))
    })
  })

  // Faders: the slot, the cap, both functions printed along the slot
  for (const f of F.FADERS) {
    const g = svg('g', { class: 'control', tabindex: 0, 'aria-label': f.index === 6 ? 'S36' : 'S37' }, board)
    svg('rect', { x: f.x - F.SLOT_W / 2, y: f.y0 - F.SLOT_W / 2, width: F.SLOT_W, height: f.y1 - f.y0 + F.SLOT_W, rx: F.SLOT_W / 2, fill: '#08080a' }, g)
    svg('rect', { x: f.x - 1.2, y: f.y0, width: 2.4, height: f.y1 - f.y0, rx: 1.2, fill: '#26262a' }, g)
    const cap = svg('g', {}, g)
    svg('rect', { x: f.x - 7, y: -5, width: 14, height: 10, rx: 2, fill: 'url(#cap)', stroke: '#2c2c30', 'stroke-width': 0.8 }, cap)
    svg('line', { x1: f.x - 5, y1: 0, x2: f.x + 5, y2: 0, class: 'pointer' }, cap)
    const side = f.index === 6 ? 1 : -1
    const lx = f.x + side * 16
    const main = svg('text', { class: 'print', x: lx, y: (f.y0 + f.y1) / 2, transform: `rotate(${-90 * side} ${lx} ${(f.y0 + f.y1) / 2})` }, board)
    const second = svg('text', { class: 'print second', x: lx + side * 9, y: (f.y0 + f.y1) / 2, transform: `rotate(${-90 * side} ${lx + side * 9} ${(f.y0 + f.y1) / 2})` }, board)
    main.textContent = MAIN[f.index]
    second.textContent = SECOND[f.index]
    labels.push([main, second])
    const set = (e) => controls.setKnob(f.index, clamp((f.y1 - point(e).y) / (f.y1 - f.y0)))
    g.addEventListener('pointerdown', (e) => (g.setPointerCapture(e.pointerId), set(e)))
    g.addEventListener('pointermove', (e) => g.hasPointerCapture(e.pointerId) && set(e))
    g.addEventListener('dblclick', () => controls.setKnob(f.index, controls.defaults[f.index]))
    describe(g, () => `S3${f.index} · ${controls.shifted ? KNOB_HELP[f.index][1] : KNOB_HELP[f.index][0]}`, readout)
    draws.push(() => cap.setAttribute('transform', `translate(0 ${f.y1 - controls.knobs[f.index] * (f.y1 - f.y0)})`))
  }

  // Switches: a toggle on the stones; click above or below its centre
  F.SWITCHES.forEach(([x, y], index) => {
    const g = svg('g', { class: 'control', tabindex: 0, 'aria-label': index === 0 ? 'S09' : 'S07' }, board)
    svg('rect', { x: x - 10, y: y - 13, width: 20, height: 39, rx: 5, fill: '#0c0c0d' }, g)
    svg('circle', { cx: x, cy: y, r: 6, fill: 'url(#metal)' }, g)
    const lever = svg('line', { x1: x, y1: y, stroke: 'url(#metal)', 'stroke-width': 3.2, 'stroke-linecap': 'round' }, g)
    const ball = svg('circle', { r: 3, fill: '#e4e4e4' }, g)
    const text = svg('text', { class: 'switch-label', x, y: y + 21 }, g)
    g.addEventListener('pointerdown', (e) => {
      const p = controls.switches[index]
      const up = point(e).y < y
      controls.setSwitch(index, up ? (p === 2 ? 0 : 1) : p === 1 ? 0 : 2)
      readout.textContent = `S0${index === 0 ? 9 : 7} · ${SWITCH_HELP[index][controls.switches[index]]}`
    })
    describe(g, () => `S0${index === 0 ? 9 : 7} · ${SWITCH_HELP[index][controls.switches[index]]}`, readout)
    draws.push(() => {
      const pos = controls.switches[index]
      const dy = pos === 1 ? -9 : pos === 2 ? 9 : 0
      const dx = pos === 0 ? 0 : 1.5
      lever.setAttribute('x2', x + dx)
      lever.setAttribute('y2', y + dy)
      ball.setAttribute('cx', x + dx)
      ball.setAttribute('cy', y + dy)
      text.textContent = SWITCH_TEXT[index][pos]
    })
  })

  return function draw(view) {
    for (const d of draws) d()
    const shifted = controls.shifted
    for (const [main, second] of labels) {
      main.classList.toggle('off', shifted)
      second.classList.toggle('on', shifted)
    }
    led.setAttribute('opacity', 0.15 + 0.85 * view.led)
    ledGlow.setAttribute('opacity', view.led)
  }
}
