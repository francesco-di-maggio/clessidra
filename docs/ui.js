// Small controls for the hourglass view: dial, slider, segmented switch, hold button.
// Each takes get / set functions, so it shows and changes one shared value.

const SVG = 'http://www.w3.org/2000/svg'
const DRAG_PX = 200 // a dial's full range
const SWEEP = 270 // degrees

export function svg(name, attrs = {}, parent) {
  const e = document.createElementNS(SVG, name)
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, v)
  parent?.append(e)
  return e
}

export function html(name, attrs = {}, parent) {
  const e = document.createElement(name)
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, v)
  parent?.append(e)
  return e
}

// The readout line: what the control under the pointer does
export function describe(element, text, readout) {
  element.addEventListener('pointerenter', () => (readout.textContent = text()))
  element.addEventListener('pointerleave', () => (readout.textContent = ' '))
  element.addEventListener('focus', () => (readout.textContent = text()))
}

// Drag up and down, the wheel, double-click resets
export function dial(parent, { label, get, set, reset, help, readout }) {
  const box = html('div', { class: 'dial', tabindex: 0, role: 'slider', 'aria-label': label }, parent)
  const s = svg('svg', { viewBox: '0 0 48 48' }, box)
  const r = 18
  const arcPath = (from, to) => {
    const a0 = ((from - 0.5) * SWEEP * Math.PI) / 180
    const a1 = ((to - 0.5) * SWEEP * Math.PI) / 180
    const large = to - from > 180 / SWEEP ? 1 : 0
    return `M ${24 + r * Math.sin(a0)} ${24 - r * Math.cos(a0)} A ${r} ${r} 0 ${large} 1 ${24 + r * Math.sin(a1)} ${24 - r * Math.cos(a1)}`
  }
  svg('path', { class: 'track', d: arcPath(0, 1) }, s)
  const value = svg('path', { class: 'value' }, s)
  const mark = svg('line', { class: 'mark' }, s)
  html('span', {}, box).textContent = label
  const draw = () => {
    const v = get()
    value.setAttribute('d', v > 0.001 ? arcPath(0, v) : '')
    const a = ((v - 0.5) * SWEEP * Math.PI) / 180
    mark.setAttribute('x1', 24 + 9 * Math.sin(a))
    mark.setAttribute('y1', 24 - 9 * Math.cos(a))
    mark.setAttribute('x2', 24 + r * Math.sin(a))
    mark.setAttribute('y2', 24 - r * Math.cos(a))
    box.setAttribute('aria-valuenow', v.toFixed(2))
  }
  let start = 0
  let y = 0
  box.addEventListener('pointerdown', (e) => {
    box.setPointerCapture(e.pointerId)
    start = get()
    y = e.clientY
  })
  box.addEventListener('pointermove', (e) => {
    if (box.hasPointerCapture(e.pointerId)) set(start - (e.clientY - y) / DRAG_PX)
  })
  box.addEventListener('dblclick', reset)
  box.addEventListener('wheel', (e) => (e.preventDefault(), set(get() - Math.sign(e.deltaY) * 0.01)), { passive: false })
  box.addEventListener('keydown', (e) => {
    const step = { ArrowUp: 0.01, ArrowRight: 0.01, ArrowDown: -0.01, ArrowLeft: -0.01 }[e.key]
    if (step) (e.preventDefault(), set(get() + step))
  })
  describe(box, () => `${label}: ${help()}`, readout)
  return draw
}

// A horizontal slider with an optional centre mark
export function slider(parent, { label, get, set, reset, help, readout, centre = false }) {
  const box = html('div', { class: 'slider', tabindex: 0, role: 'slider', 'aria-label': label }, parent)
  const name = html('span', { class: 'name' }, box)
  const track = html('div', { class: 'track' }, box)
  if (centre) html('i', { class: 'centre' }, track)
  const fill = html('b', {}, track)
  const knob = html('em', {}, track)
  const at = (e) => {
    const r = track.getBoundingClientRect()
    return (e.clientX - r.left) / r.width
  }
  box.addEventListener('pointerdown', (e) => {
    box.setPointerCapture(e.pointerId)
    set(at(e))
  })
  box.addEventListener('pointermove', (e) => box.hasPointerCapture(e.pointerId) && set(at(e)))
  box.addEventListener('dblclick', reset)
  box.addEventListener('keydown', (e) => {
    const step = { ArrowRight: 0.01, ArrowUp: 0.01, ArrowLeft: -0.01, ArrowDown: -0.01 }[e.key]
    if (step) (e.preventDefault(), set(get() + step))
  })
  describe(box, () => `${typeof label === 'function' ? label() : label}: ${help()}`, readout)
  return () => {
    const v = get()
    name.textContent = typeof label === 'function' ? label() : label
    fill.style.width = `${v * 100}%`
    knob.style.left = `${v * 100}%`
  }
}

// Three positions, as a three-position switch
export function segmented(parent, { label, options, get, set, help, readout }) {
  const box = html('div', { class: 'segmented', role: 'radiogroup', 'aria-label': label }, parent)
  const buttons = options.map(([value, text]) => {
    const b = html('button', { type: 'button', role: 'radio' }, box)
    b.textContent = text
    b.addEventListener('click', () => set(value))
    describe(b, () => `${label}: ${help(value)}`, readout)
    return [value, b]
  })
  return () => {
    for (const [value, b] of buttons) b.setAttribute('aria-checked', String(get() === value))
  }
}

// Held while pressed, like a pad
export function holdButton(parent, { label, press, release, isOn, help, readout }) {
  const b = html('button', { type: 'button', class: 'hold' }, parent)
  b.textContent = label
  b.addEventListener('pointerdown', (e) => {
    b.setPointerCapture(e.pointerId)
    press(`p${e.pointerId}`)
  })
  const up = (e) => release(`p${e.pointerId}`)
  b.addEventListener('pointerup', up)
  b.addEventListener('pointercancel', up)
  describe(b, () => `${label}: ${help}`, readout)
  return () => b.classList.toggle('on', isOn())
}
