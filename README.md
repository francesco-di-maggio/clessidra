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
| P10 | Hold to record |
| P11 | Hold for the second layer, below |
| LED | Flashes at 4 Hz while recording, otherwise blinks per grain start |

Second layer, while P11 is held. After a layer change, a knob moves its value from where it is, and both reach the end stops together.

| Control | Function |
|---|---|
| S31 | Window shape: near-rectangle / sine (centre, default) / Hann |
| S32 | Stereo spread, mono – full width. Default: half. |
| S34 | Reverb amount and decay. Default: none. |
| S35 | Reverb brightness, 2–18 kHz. Default: 10 kHz. |
| S36 | Dry / wet, equal power. Default: half. |
| S37 | Output gain, silent – unity. Default: unity. |

## Audio

| | |
|---|---|
| Input | Stereo, recorded as mono (left + right) |
| Output | Stereo, peak-limited at −0.5 dBFS |
| Sample rate | 48 kHz |
| Recording | Up to 30 s |

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
