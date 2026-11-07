# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

One layer: every control does one thing, where it is. The recording is the sand, the selection how much of it is in the glass.

| Control | Name | Does |
|---|---|---|
| S36 | Speed | The time stretched, the pitch kept. Held at the centre. Up forward, down reverse, with stops at 1, 2 and 4× |
| S37 | Density | Grains at once, 1/16 to 16, as Clouds: per second they follow Size. None at the centre. Down: regular, drops to a stream. Up: random, scattered to a cloud. 32 voices: with all sounding, a new grain is dropped. The reverb follows, more for a lone grain |
| S30 | Position | Where the sand starts, anywhere in the recording. Held: moves the point |
| S35 | Length | How much sand: the selection, past the end round to the start. The playhead stays where it is |
| S31 | Size | 5 ms to 2 s, 100 ms at the centre, at most half the recording |
| S32 | Spray | How far grains stray from the playhead, evenly: places up to the whole selection, sizes, pitch up to ±12 semitones, pan, levels (to −18 dB), reverb sends |
| S33 | Pitch | ±24 semitones, stops at 0, 1, 3, 4, 7, 12, 24 (Beads) |
| S34 | Shape | The grain's window: reverse percussive down, Hann at the centre, percussive up |
| S07 | End | When the playhead runs out: up stops, centre turns back, down loops |

| Pad | Name | Does |
|---|---|---|
| P10 | Record | Tap: the last 8 s. Hold: until release, up to 30 s, the input heard. A new recording pours in over 2 s |
| P00 | Seed | A touch: one grain at the playhead, whatever Density. At Density's centre, every grain is yours |
| P01 | Mute | Held: the grains silent, the reverb rings on |
| P02 | Restart | The sand back to the top: the playhead to the selection start |

Not assigned yet: S09, P03–P09, P11.

| LED | |
|---|---|
| Glow | Grains falling, dimmer as the sand runs out |
| Flash | Recording |
| Pulse | At a stop: unison, speed 1× either way, 100 ms, Hann |

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
