// The Simple Touch in hundredths of an inch, y down. The overlay (outline, holes) is
// Synthux's faceplate template (AudreyTouch, Faceplate/Simple Touch Faceplate
// template.dxf, MIT); pads and switches are placed from SynthuxSimpleTouch_layout.jpg
// at 188.4 px per inch, the scale at which it matches the template.

export const BOARD = { x: -9.3, y: -84.4, width: 318.5, height: 516.5 }
export const OVERLAY = 'M89.2 -43.8 L98.2 -42.9 L102.6 -42.0 L109.1 -40.1 L113.3 -38.5 L119.3 -35.6 L125.0 -32.0 L128.6 -29.4 L133.7 -24.9 L138.4 -19.9 L172.1 24.1 L176.5 29.3 L181.4 34.1 L186.7 38.2 L192.3 41.8 L198.3 44.8 L202.5 46.5 L208.9 48.5 L215.5 49.9 L220.0 50.4 L225.6 50.6 L305.0 50.6 L306.7 50.9 L308.5 52.2 L309.3 53.7 L309.6 55.2 L309.6 219.3 L309.2 221.0 L308.0 222.7 L306.5 223.6 L305.0 223.8 L-4.3 223.8 L-6.0 223.5 L-7.8 222.2 L-8.7 220.8 L-8.9 219.3 L-8.9 -39.3 L-8.6 -41.0 L-8.1 -42.0 L-6.9 -43.1 L-4.3 -43.9 Z'
export const OVERLAY_BOTTOM = 223.8

// Knob holes: radius 17.7. Index = Controls::Knob
export const KNOB_R = 17.7
export const KNOBS = [
  [58.6, 168.9], // S30
  [58.6, 101.4], // S31
  [119.7, 101.4], // S32
  [180.8, 101.4], // S33
  [241.9, 101.4], // S34
  [241.9, 168.9], // S35
]
// Fader slots: centre x, top and bottom of travel, width
export const FADERS = [
  { index: 6, x: 11.1, y0: 102.5, y1: 196.8 }, // S36
  { index: 7, x: 289.4, y0: 102.5, y1: 196.8 }, // S37
]
export const SLOT_W = 16.9
export const JACKS = [
  { name: 'In', x: 11.0, y: -14.7 },
  { name: 'Out', x: 11.0, y: 45.3 },
]
export const JACK_R = 15.8
export const SCREWS = [[11.1, 84.0], [289.4, 84.0], [11.1, 206.0], [289.4, 206.0], [288.9, 62.1]]
export const SCREW_R = 8.9
// Pads P00-P11, then the switches S09 and S07
export const PADS = [
  [73.0, 243.7], // P00
  [154.7, 239.4], // P01
  [210.4, 245.8], // P02
  [18.3, 340.3], // P03
  [80.4, 321.7], // P04
  [144.6, 337.6], // P05
  [212.6, 333.9], // P06
  [289.5, 357.8], // P07
  [94.7, 399.7], // P08
  [213.6, 402.4], // P09
  [118.6, -58.9], // P10
  [192.4, -58.9], // P11
]
export const SWITCHES = [
  [106.4, 253.8], // S09
  [165.9, 281.9], // S07
]
