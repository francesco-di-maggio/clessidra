# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass. In progress.

## Controls

<img src="touch.jpeg" width="300"/>

Current mapping, for testing the engine.

| Control | Function |
|---|---|
| S30 / S35 | Playback start / end, 0–100 % of the recording |
| S31 | Grain size, 20 ms – 2 s. Centre detent: 100 ms. |
| S32 | Random position |
| S33 | Pitch, ±2 octaves. Flat at unison, fifths and octaves. |
| S34 | Random pitch |
| S36 | Playback speed, 0–4. Centre detent: 1. Position while frozen. |
| S37 | Flow with no pad touched, 0.1–16 grains at once |
| S09 | Playback: forward / freeze / reverse |
| S07 | Grain starts: periodic / half random / random |
| P00, P02–P09 | Pressure raises flow to 16 grains at once |
| P10 | Hold to record, up to 30 s |
| P11 | Hold for the second layer, below |
| LED | On while recording, otherwise a blink per grain start |

Second layer, while P11 is held. After a layer change, a knob moves its value from where it is, and both reach the end stops together.

| Control | Function |
|---|---|
| S31 | Window shape: near-rectangle / sine (centre, default) / Hann |
| S32 | Stereo spread, mono – full width. Default: half. |
| S34 | Chance that a grain plays, 0–100 %. Default: 100 %. |
| S37 | Output gain, silent – unity. Default: unity. |

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
