# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

One layer: every control does one thing, where it is. The recording is the sand, the selection how much of it is in the glass.

| Control | Name | Does |
|---|---|---|
| S36 | Speed | Held at the centre. Up forward, down reverse, with stops at 1, 2 and 4× |
| S37 | Density | None at the bottom, then 0.3 to 131 grains per second. Size (30 to 300 ms) and reverb (more for a lone grain) follow. Near the top the cloud sprays by itself |
| S30 | Position | Where the sand starts, anywhere in the recording. Held: moves the point |
| S35 | Length | How much sand: the selection, past the end round to the start. The playhead stays where it is |
| S31 | Size | A quarter to four times Density's size, up to half the recording |
| S32 | Spray | Plain and periodic at 0. Up: random starts, places, sizes, shapes, wider pan, mostly near with a long tail. Top quarter: random pitch, up to ±12 semitones |
| S33 | Pitch | ±24 semitones, stops at 0, 1, 3, 4, 7, 12, 24 (Beads) |
| S34 | Shape | The grain's window: reverse percussive down, Hann at the centre, percussive up |
| S07 | End | When the playhead runs out: up stops, centre turns over by itself, down loops |
| S09 | Scale | The notes' scale: up major, centre minor, down major pentatonic |

| Pad | Name | Does |
|---|---|---|
| P10 | Record | Tap: the last 8 s. Hold: until release, up to 30 s, the input heard. A new recording pours in over 2 s |
| P11 | Shake | Held: Spray towards the top, firmer for more. Let go, it settles |
| P00 | Seed | A touch: one grain at the playhead, whatever Density, as Beads' Seed |
| P01 | Mute | Held: the grains silent, the reverb rings on |
| P02 | Restart | The sand back to the top: the playhead to the selection start |
| P03–P09 | Notes | Held: grains at the playhead, the degrees of S09's scale above Pitch, lowest to highest. Density at least half, pressure raises it. Together with the playhead at most 131 grains per second. Silent once the playhead has run out |

| LED | |
|---|---|
| Glow | Grains falling, dimmer as the sand runs out |
| Flash | Recording |
| Pulse | At a stop: unison, speed 1× either way, Density's size, Hann |

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
