# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

One layer: every control does one thing, where it is. Record a sound: it is the sand, the selection how much of it is in the glass.

| Control | Name | Does |
|---|---|---|
| S36 | Speed | How fast the sand runs: the time stretched, the pitch kept. Held at the centre. Up forward, down reverse, with stops at ¼, ½, 1, 2 and 4× |
| S37 | Density | Grains at once, 1/16 to 16: per second they follow Size. None at the centre. Down: regular, drops to a stream. Up: random, scattered to a cloud. 32 voices: with all sounding, a new grain is dropped |
| S30 | Position | Where the sand starts, anywhere in the recording. Held: moves the point |
| S35 | Length | How much sand: the selection, past the end round to the start. The playhead stays where it is |
| S31 | Size | 5 ms to 2 s, 100 ms at the centre, at most half the recording |
| S32 | Spray | How far grains stray from the playhead, evenly: position up to the whole selection, size, pitch up to ±12 semitones, pan, level (to −18 dB), reverb send |
| S33 | Pitch | ±24 semitones, stops at 0, 1, 3, 4, 7, 12, 24 |
| S34 | Shape | The grain's window: reverse percussive down, Hann at the centre, percussive up |
| S09 | Reverb | On the grains: down off, centre short (1.7 s), up long (8 s) |
| S07 | End | When the sand runs out: up stops, centre turns the glass over, down starts again |

| Pad | Name | Does |
|---|---|---|
| P10 | Record | Tap: the last 8 s. Hold: up to 30 s, the input heard. The new sand pours in over 2 s |
| P11 | Turn | A touch turns the glass over: from where it is, the sand runs the other way, the grains backwards |
| P00 | Shake | Held: Spray at its top |
| P01 | Mute | Held: the grains silent, the reverb rings on |
| P02 | Hold | Held: the sand stops, the grains stay on that moment |
| P03–P09 | Places | The selection in sevenths, left to right on the board: P03, P04, P08, P05, P09, P06, P07. A touch: the playhead there, a grain at once, the beat from it. Held: it stays there |

| LED | |
|---|---|
| Glow | Grains falling, dimmer as the sand runs out |
| Flash | Recording |
| Pulse | At a stop: unison, speed 1× either way, 100 ms, Hann |

| To play | Set |
|---|---|
| Rhythm | Density below the centre, Shape up, Size short. Tap the places: each restarts the beat |
| Drums by hand | Density at the centre: each place plays one grain |
| Cloud | Density at the top, Spray up, Size long, Reverb long |
| Stretch | Speed at ¼ or ½, Density up. Turn to hear it backwards |
| A moment held | Hold, or a place held, Density up, a little Spray, Shake for a burst |

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
