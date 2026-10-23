// The instrument's controls, shared by the view and the keys: knob positions,
// second-layer values, switches and pads. Every change goes to the engine; the
// mapping from controls to sound is the firmware's own (ui/clessidra_ui.cpp).

const PAD_START = 0.2 // pressure at the touch
const PAD_SWELL_S = 1.5 // a held pad reaches full pressure
const MIN_TOUCH_MS = 20 // a shorter touch is released this late, so the engine sees it

// Knob and fader index = Controls::Knob (ui/clessidra_ui.h)
export const KNOB = { S30: 0, S31: 1, S32: 2, S33: 3, S34: 4, S35: 5, S36: 6, S37: 7 }
// Switch positions as libDaisy's Switch3
export const POS = { centre: 0, up: 1, down: 2 }
// Pads
export const PAD = { stop: 2, record: 10, shift: 11 }
export const PLACE_PADS = [3, 4, 8, 5, 6, 9, 7] // places first to last (ui/clessidra_ui.h)

// What each control does: [main layer, second layer]. Shown in the readout.
export const KNOB_HELP = [
  ['Selection start', null],
  ['Grain size, 10 ms – 4 s', 'Window: reverse percussive / Hann / percussive'],
  ['Random position', null],
  ['Pitch, ±2 octaves, flat at unison, fifths and octaves', 'Harmonic: share of random pitches on partials'],
  ['Random pitch, up to ±2 octaves', 'Reverb amount and decay'],
  ['Selection end', 'Tone: lowpass / off / highpass'],
  ['Speed, 0–4, centre 1. Scrubs while frozen.', 'Dry / wet'],
  ['Flow: grains at once, off to 16', 'Resample: what Record keeps, input to grains'],
]
export const SWITCH_HELP = [
  { 1: 'Forward', 0: 'Freeze', 2: 'Reverse' },
  { 1: 'Periodic grain starts', 0: 'Half random grain starts', 2: 'Random grain starts' },
]

export class Controls {
  constructor(send) {
    this.send = send
    this.knobs = [0, 0.5, 0, 0.5, 0, 1, 0.5, 0.8]
    this.defaults = [...this.knobs]
    // Second layer, as the engine's defaults: window Hann, harmonic none, reverb none,
    // tone off, dry/wet half, resample none
    this.second = [null, 0.5, null, 0, 0, 0.5, 0.5, 0]
    this.secondDefaults = [...this.second]
    this.switches = [POS.up, POS.centre]
    this.held = new Map() // who (pointer, key) -> pad
    this.since = new Map() // pad -> touch time
    this.pen = new Map() // pad -> a pen's pressure
    this.listeners = new Set()
    this.secondEdited = 0 // time of the last change here, so the engine's echo doesn't fight a drag
  }

  // Everything to the engine, as it is now
  sync() {
    this.knobs.forEach((value, index) => this.send({ type: 'knob', index, value }))
    this.second.forEach((value, index) => value !== null && this.send({ type: 'second', index, value }))
    this.switches.forEach((position, index) => this.send({ type: 'switch', index, position }))
  }

  setKnob(index, value) {
    this.knobs[index] = clamp(value)
    this.send({ type: 'knob', index, value: this.knobs[index] })
    this.changed()
  }

  setSecond(index, value) {
    this.second[index] = clamp(value)
    this.secondEdited = performance.now()
    this.send({ type: 'second', index, value: this.second[index] })
    this.changed()
  }

  setSwitch(index, position) {
    this.switches[index] = position
    this.send({ type: 'switch', index, position })
    this.changed()
  }

  press(pad, who, pen = null) {
    if (this.held.has(who)) return
    this.held.set(who, pad)
    if (!this.since.has(pad)) this.since.set(pad, performance.now())
    if (pen !== null) this.pen.set(pad, pen)
    // At once: the frame loop only updates pressure, and pauses in a hidden page
    this.send({ type: 'pad', index: pad, touched: true, pressure: this.pressure(pad) })
    this.changed()
  }

  release(who) {
    const pad = this.held.get(who)
    if (pad === undefined) return
    this.held.delete(who)
    if (this.isHeld(pad)) return
    const late = MIN_TOUCH_MS - (performance.now() - this.since.get(pad))
    this.since.delete(pad)
    this.pen.delete(pad)
    const off = () => !this.isHeld(pad) && this.send({ type: 'pad', index: pad, touched: false, pressure: 0 })
    if (late > 0) setTimeout(off, late)
    else off()
    this.changed()
  }

  releaseAll() {
    for (const who of [...this.held.keys()]) this.release(who)
  }

  isHeld(pad) {
    return [...this.held.values()].includes(pad)
  }

  pressure(pad) {
    if (!this.since.has(pad)) return 0
    if (this.pen.has(pad)) return this.pen.get(pad)
    const s = (performance.now() - this.since.get(pad)) / 1000
    return Math.min(1, PAD_START + (s / PAD_SWELL_S) * (1 - PAD_START))
  }

  // Every frame: the held pads' pressure
  tick() {
    for (const pad of new Set(this.held.values())) {
      this.send({ type: 'pad', index: pad, touched: true, pressure: this.pressure(pad) })
    }
  }

  get shifted() {
    return this.isHeld(PAD.shift)
  }

  onChange(f) {
    this.listeners.add(f)
  }

  changed() {
    for (const f of this.listeners) f()
  }
}

export const clamp = (x) => Math.min(1, Math.max(0, x))
