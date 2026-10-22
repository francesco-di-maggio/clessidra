# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

| Control | Function |
|---|---|
| S30 / S35 | Selection start / end, 0–100 % of the recording, either order |
| S31 | Grain size, 20 ms – 2 s. Centre detent: 100 ms. |
| S32 | Random position |
| S33 | Pitch, ±2 octaves. Flat at unison, fifths and octaves. |
| S34 | Random pitch |
| S36 | Playback speed, 0–4. Centre detent: 1. Scrubs while frozen. |
| S37 | Playhead flow: off, then 0.1–16 grains at once |
| S09 | Playback: forward / freeze / reverse |
| S07 | Grain starts: periodic / half random / random |
| P03, P00, P04, P08, P05, P02, P06, P09, P07 | Places: ninths of the selection, first to last in this order. Pressure sets the flow, 1–16 grains at once. |
| P01 | Hold to stop the flow |
| P10 | Record. Tap: the last 10 s. Hold: from just before the touch to the release, up to the last 10 s. Silence at both ends is cut; a silent recording changes nothing. Grains playing finish on the previous recording. |
| P11 | Hold for the second layer, below |
| LED | Flashes at 4 Hz while P10 is touched. Pulses at each centre: pitch unison, speed 1, grain size 100 ms, tone off. Steady while P11 is held: brightness shows the value of the second-layer knob last turned (at power-up: dry/wet). Otherwise glows dimly with the grains: each fades in and out with its window, overlapping grains glow steady. |

Second layer, while P11 is held. After a layer change, a knob moves its value from where it is, and both reach the end stops together.

| Control | Function |
|---|---|
| S31 | Window shape: near-rectangle / sine (centre, default) / Hann |
| S32 | Stereo spread, mono – full width. Default: half. |
| S34 | Reverb amount and decay. Default: none. |
| S35 | Tone, on the grains before the reverb: lowpass down to 500 Hz / off (centre detent, default) / highpass up to 2 kHz |
| S36 | Dry / wet, equal power. Default: half. |
| S37 | Resample: what P10 records, input – wet (grains after tone and reverb), equal power. Default: input. |

## Audio

| | |
|---|---|
| Input | Stereo, recorded as mono (left + right) |
| Output | Stereo, peak-limited at −0.5 dBFS |
| Sample rate | 48 kHz |
| Recording | Up to 10 s, from an input ring always listening |
| Grains | Each starts within 12 ms of its position, where it best continues the previous grain (WSOLA): smooth stretching and pitch shifting |

## Quick Install

Download the [binary](https://github.com/francesco-di-maggio/clessidra/releases/latest/download/clessidra.bin) and flash it with the [Daisy Seed web programmer](https://electro-smith.github.io/Programmer/).

## Prerequisites

- [Daisy Toolchain](https://daisy.audio/tutorials/cpp-dev-env/) (ARM GCC + make)

## Build

```shell
git clone --recurse-submodules https://github.com/francesco-di-maggio/clessidra.git
cd clessidra
make libs
make
```

## Flash

Hold BOOT, press and release RESET, release BOOT, then:

```shell
make program-dfu
```

Serial log over USB (controls, recording length, CPU load):

```shell
make clean
make LOG=1
make program-dfu
```

## License

MIT
