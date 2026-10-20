# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass. In progress.

## Controls

Current mapping, for testing the engine.

| Control | Function |
|---|---|
| S30 / S35 | Playback start / end, 0–100 % of the recording |
| S31 | Grain size, 10 ms to half the recording. Centre detent. |
| S32 | Random position |
| S33 | Pitch, 0.01–2. Centre detent: 1. |
| S34 | Random pitch |
| S36 | Playback speed, 0–4. Centre detent: 1. |
| S37 | Gain |
| P10 | Hold to record, up to 30 s |
| LED | On while recording |

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
