# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

| Control | Function |
|---|---|
| S36 | Tilt: centre stops, up forward, down backward, up to 4 times. Speed 1 half way |
| S37 | Pour: closed, single percussive drops in reverb, a stream, a wide cloud of long grains. Up to 130 grains per second |
| S30 | Position: how far back the selection lies from where the sound ends |
| S35 | Length of the selection |
| S31 | Grain size, 2 octaves either way from what Pour sets |
| S32 | Scatter: from periodic and plain to random starts, positions and sizes. Above the centre, random pitch |
| S33 | Pitch, ±2 octaves, stops at 0, 0.1, 1, 3, 4, 7, 12 and 24 semitones |
| S34 | Fill: how much of the input replaces the sound, and the odds that a grain follows the input. Down: sealed, the sound stays. Up: you hear the input through the grains, about 50 ms late |
| S07 | End: one-shot / turn / loop |
| P03–P09 | Slices at the sound's onsets. A touch plays one, a firm press opens Pour |

| LED | |
|---|---|
| Glow | Grains playing |
| Pulse | A control at or turning within its centre |
| Steady | P11 or P00 held: brightness shows the value |

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
