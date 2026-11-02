// What clessidra.wasm hands the page, laid out as web/clessidra_web.cpp lays
// it out. One place: the worklet, the page and the tests read it from here.

export const RATE = 48000; // config.h kSampleRateHz
export const BLOCK = 128; // Web Audio's render quantum
export const NUM_SLICES = 7; // onsets.h kNumSlices
export const MAX_RECORD_S = 30; // config.h kBufferSeconds
export const RECORD_PAD = 10; // CoreUI::kRecordPad
export const LAYER_PAD = 11; // CoreUI::kLayerPad
export const RESTART_PAD = 2; // CoreUI::kRestartPad
export const SLICE_PADS = [3, 4, 8, 5, 6, 9, 7]; // CoreUI::kSlicePads
// The incoming strip: the last seconds a tap keeps (Core::kTapCaptureSeconds)
export const INCOMING_SECONDS = 8;
export const INCOMING_COLUMNS = 320;

// cl_view, in samples of the recording
export const VIEW = { sound: 0, selStart: 1, selLength: 2, playhead: 3, edges: 4, strikes: 4 + NUM_SLICES + 1 } as const;
export const VIEW_SIZE = VIEW.strikes + NUM_SLICES;
// cl_params, in the core's units
export const PARAM = { speed: 0, freeze: 1, density: 2, grainMs: 3, pitch: 4, reverb: 5, jitter: 6, randPitch: 7, shape: 8, start: 9, length: 10, mix: 11, tone: 12, randPos: 13, randSize: 14, randShape: 15 } as const;
export const PARAM_SIZE = 16;
// cl_grain_fields, per grain: position 0..1 of the recording, progress, window shape, pan
export const GRAIN_FIELDS = 4;

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
  cl_second(knob: number, value: number): void;
  cl_led(nowMs: number): number;
  cl_peaks(): number;
  cl_columns(): number;
  cl_recordings(): number;
  cl_view(): number;
  cl_grains(): number;
  cl_grain_fields(): number;
  cl_params(): number;
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
}

// What the page sends: Controls' knobs, switches and pads
export type Control =
  | { type: "knob"; index: number; value: number }
  | { type: "second"; index: number; value: number }
  | { type: "switch"; index: number; position: number }
  | { type: "pad"; index: number; touched: boolean; pressure: number };

export type FromWorklet = Frame | { type: "ready" } | { type: "error"; message: string };
