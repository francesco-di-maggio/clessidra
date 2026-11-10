// What clessidra.wasm hands the page, laid out as web/clessidra_web.cpp lays
// it out. One place: the worklet, the page and the tests read it from here.

export const RATE = 48000; // config.h kSampleRateHz
export const BLOCK = 128; // Web Audio's render quantum
export const MAX_RECORD_S = 30; // config.h kBufferSeconds
// CoreUI's pads
export const STOP_PAD = 0;
export const RESET_PAD = 1; // with Shift
export const TRIG_PAD = 2;
export const SLICE_PADS = [3, 4, 8, 5, 9, 6, 7]; // P03–P09, left to right
export const RECORD_PAD = 10;
export const SHIFT_PAD = 11;
export const VOICES = 32; // Core::kNumVoices
export const TAP_SECONDS = 8; // what a tap keeps, Core::kTapCaptureSeconds
// The incoming strip: the input over the longest of them
export const INCOMING_SECONDS = 16;
export const INCOMING_COLUMNS = 1280;

// cl_view, in samples of the recording
export const VIEW = { sound: 0, selStart: 1, selLength: 2, playhead: 3 } as const;
export const VIEW_SIZE = 4;
// cl_params, in the core's units. grainMs: the size in use, grainMsSet: as
// set. randTime: starts, 0 regular to 1 random. randPitch: below 0 in
// octaves, above free. volume: linear. spread: pan width, 0..1. filter: -1
// low-pass, 0 off, 1 high-pass. end: End, 0 once, 1 turn, 2 loop.
export const PARAM = { speed: 0, freeze: 1, density: 2, grainMs: 3, pitch: 4, reverb: 5, randTime: 6, randPitch: 7, shape: 8, start: 9, length: 10, volume: 11, randPos: 12, randSize: 13, grainMsSet: 14, minGrainMs: 15, maxGrainMs: 16, spread: 17, filter: 18, end: 19, stop: 20 } as const;
export const PARAM_SIZE = 21;
export const SLICES = 7; // Onsets::kNumSlices
// cl_grain_fields, per grain: position 0..1 of the recording, progress, window
// shape, pan, pitch in semitones, size in ms
export const GRAIN_FIELDS = 6;

export interface Exports {
  memory: WebAssembly.Memory;
  _initialize?: () => void;
  cl_init(sampleRate: number): number;
  cl_input(channel: number): number;
  cl_output(channel: number): number;
  cl_process(size: number, nowMs: number): void;
  cl_knob(knob: number, value: number): void;
  cl_switch(which: number, position: number): void;
  cl_pad(pad: number, touched: number, pressure: number): void;
  cl_led(nowMs: number): number;
  cl_peaks(): number;
  cl_columns(): number;
  cl_recordings(): number;
  cl_view(): number;
  cl_grains(): number;
  cl_grain_fields(): number;
  cl_params(): number;
  cl_slices(): number;
  cl_chunk(): number;
  cl_chunk_size(): number;
  cl_feed(length: number): void;
  cl_capture(length: number): void;
}

// What the worklet sends, once per screen frame
export interface Frame {
  type: "view";
  led: number;
  input: number; // peak since the last frame, 0..1
  output: number;
  incoming: Float32Array; // the input's peaks over INCOMING_SECONDS, oldest first
  view: Float32Array;
  grains: Float32Array;
  params: Float32Array;
  peaks: Float32Array;
  slices: Float32Array; // where each slice starts in the loop, 0..1
}

// What the page sends: Controls' knobs, switches and pads
export type Control =
  | { type: "knob"; index: number; value: number }
  | { type: "switch"; index: number; position: number }
  | { type: "load"; index: -1; samples: Float32Array } // a file into the recording, mono, RATE
  | { type: "pad"; index: number; touched: boolean; pressure: number };

export type FromWorklet = Frame | { type: "ready" } | { type: "error"; message: string };
