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
| S36 | Playback speed, 0–4. Centre detent: 1. Position while frozen. |
| S37 | Playhead flow, 0.1–16 grains at once |
| S09 | Playback: forward / freeze / reverse |
| S07 | Grain starts: periodic / half random / random |
| P03, P00, P04, P08, P05, P02, P06, P09, P07 | Places: each pours grains from its ninth of the selection, first to last in this order. Pressure sets the flow, 1–16 grains at once. |
| P01 | Hold to stop the flow: no new grains, the sound rings out |
| P10 | Hold to record, up to 30 s. Silent until the first recording. |
| P11 | Hold for the second layer, below |
| LED | Flashes at 4 Hz while recording, otherwise blinks per grain start |

Second layer, while P11 is held. After a layer change, a knob moves its value from where it is, and both reach the end stops together.

| Control | Function |
|---|---|
| S30 | Dry / wet, equal power. Default: wet. |
| S31 | Window shape: near-rectangle / sine (centre, default) / Hann |
| S32 | Stereo spread, mono – full width. Default: half. |
| S33 | Reverb amount and decay. Default: none. |
| S34 | Chance that a grain plays, 0–100 %. Default: 100 %. |
| S37 | Output gain, silent – unity. Default: unity. |

## Audio

| | |
|---|---|
| Input | Mono: left + right |
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
