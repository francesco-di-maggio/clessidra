# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

Three layers: own, P11 held (Shift: each knob's randomness), P00 held (FX). A knob takes over a value once it reaches it.

| Control | Own | P11 Shift | P00 FX |
|---|---|---|---|
| S30 | Position: selection start | Position random, per grain | |
| S35 | Length: selection length, past the end round to the start | Length random, per pass | |
| S31 | Size: 10 ms to 4 s, 200 ms at the centre | Size random | Filter: low-pass left, off at the centre, high-pass right |
| S32 | Shape: reverse percussive, Hann at the centre, percussive | Shape random | Reverb |
| S33 | Pitch: ±24 semitones, stops at 0, 0.1, 5, 7, 12, 17, 19, 24 | Pitch random | |
| S34 | Density: none at the bottom, 0.3 to 131 grains per second | Start randomness: periodic at the centre | |
| S36 | Speed: 0 to 4×, 1× at the centre, stopped at the bottom | | |
| S37 | Dry/Wet, equal power | | |

Randomness, as Beads: off at the centre, up an even spread, down mostly near with a long tail.

| Control | Function |
|---|---|
| P10 | Record. Tap: the last 8 s. Hold: until release, up to 30 s |
| P11 | Shift layer, held |
| P00 | FX layer, held |
| P02 | Restart the playhead |
| P03–P09 | Slices at the recording's onsets. Each plays its own grains. Pressure sets their density. With S34 at the bottom, only the slices sound |
| S09 | Direction: forward, freeze, reverse |
| S07 | Trigger: once, while held, looped while held |

| LED | |
|---|---|
| Glow | Grains playing |
| Flash | Recording |
| Pulse | At a centre: pitch unison, speed 1×, size 200 ms |
| Steady | P11 or P00 held: brightness shows the value |

## Play in the Browser

[francescodimaggio.nl/clessidra](https://francescodimaggio.nl/clessidra/): the same firmware, compiled to WebAssembly.

## Install

Download the [binary](https://github.com/francesco-di-maggio/clessidra/releases/latest/download/Clessidra.bin) and flash it with the [Daisy Seed web programmer](https://electro-smith.github.io/Programmer/).

## Build

Needs the [Daisy Toolchain](https://daisy.audio/tutorials/cpp-dev-env/).

```shell
git clone --recurse-submodules https://github.com/francesco-di-maggio/clessidra.git
cd clessidra
make libs
make
```

Hold BOOT, press and release RESET, release BOOT, then:

```shell
make program-dfu
```

## License

MIT
