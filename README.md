# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

Two layers: own, and P11 held (Shift: each knob's randomness, the reverb). A knob takes over a value once it reaches it.

| Control | Own | P11 Shift |
|---|---|---|
| S30 | Position: selection start | Position random, per grain |
| S35 | Length: selection length, past the end round to the start. The playhead stays where it is | Length random, per pass |
| S31 | Size: 10 ms, 200 ms at the centre, up to half the recording, 4 s at most | Size random |
| S32 | Shape: reverse percussive, Hann at the centre, percussive | Shape random |
| S33 | Pitch: ±24 semitones, stops at 0, 0.1, 5, 7, 12, 17, 19, 24 | Pitch random |
| S34 | Density: none at the bottom, 0.3 to 131 grains per second | Start randomness: periodic at the centre |
| S36 | Speed: 0 to 4×, 1× at the centre, stopped at the bottom. Frozen: moves the point through the selection, the speed taken up again after | |
| S37 | Filter: low-pass down, off at the centre, high-pass up | Reverb |

Randomness, as Beads: off at the centre, up an even spread, down mostly near with a long tail.

| Control | Function |
|---|---|
| P10 | Record. Tap: the last 8 s. Hold: until release, up to 30 s. A new recording pours in over 2 s |
| P11 + P10 | Listen: the input passes, firmer for more. Nothing recorded |
| P11 + P01 | Reset: every randomness off, the reverb back to 25 % |
| P11 | Shift layer, held |
| P00 | Mute, held: the grains silent, the reverb rings on |
| P02 | Retrig: the playhead to the selection start |
| P03–P09 | Slices at the recording's onsets. Each plays its own grains. Pressure sets their density. With S34 at the bottom, only the slices sound |
| S09 | Direction: forward, freeze, reverse. Frozen, the point follows S30 and holds when S35 moves |
| S07 | Trigger: once, while held, looped while held |

| LED | |
|---|---|
| Glow | Grains playing |
| Flash | Recording |
| Pulse | At a centre: pitch unison, speed 1×, size 200 ms |
| Steady | P11 held: brightness shows the value |

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
