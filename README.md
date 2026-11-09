# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

A granular sampler. Record a sound, then play it as grains: Speed moves through it, Density sets how they fall. The input is always captured, so a tap on Rec keeps the moment that just passed. Two layers: each knob alone, and its twin with P11 held. After a layer change a knob moves its value from where it is, so nothing jumps.

| Control | Alone | P11 held |
|---|---|---|
| S36 | Speed: 0 to 4×, stops at ¼, ½, 1, 2 and 4×, the pitch kept. Frozen: scrubs through the loop, a full stroke the whole loop, the speed taken up again after | Mix: the input against the grains, dry to wet |
| S37 | Density: grains at once, 1/16 to 16, per second they follow Size. None at the centre. Down: regular, drops to a stream. Up: random, scattered to a cloud. 32 voices: with all sounding, a new grain is dropped | Reverb: none to most, decay 0.5 to 16 s |
| S30 | Position: where the loop starts, anywhere in the recording. The playhead keeps its place in the loop, so it moves with it, playing or frozen | |
| S35 | Length: a share of what is left after Position, at the top to the recording's end | |
| S31 | Size: 5 ms to 2 s, 100 ms at the centre, at most half the recording | Random size, up to about two octaves either way |
| S32 | Random position: grains stray from the playhead, evenly, up to the whole loop, across the stereo field with it | |
| S33 | Pitch: ±24 semitones, stops at 0, 1, 3, 4, 7, 12, 24 | Random pitch, up to ±12 semitones |
| S34 | Shape: the grain's window, reverse percussive, Hann at the centre, percussive | Filter: low-pass to 200 Hz, off at the centre, high-pass to 2 kHz |
| S09 | Mode: up forward, centre freeze, down reverse | |
| S07 | Quantize, the random pitch: down off, centre semitones, up octaves | |

| Pad | Alone | P11 held |
|---|---|---|
| P10 | Record. The input is always captured. Tap: the last 8 s. Hold: up to 30 s, the input heard while held. Silence at the ends is trimmed, the new recording pours in over 2 s, its slices are found | Listen: the input heard, nothing recorded |
| P11 | Shift, held | |
| P00 | Stop, as a tape machine: everything slows to a halt over 0.6 s, the pitch falling. Touch again to start | |
| P01 | Reset: the twins back to their defaults | |
| P02 | Cue: back to the start of the loop, with a grain at once | |
| P03–P09 | Slices: the loop in seven, its start then the recording's strongest attacks inside it (even parts without attacks), left to right on the board (P03, P04, P08, P05, P09, P06, P07). Held: plays its slice from the start, round it, at Speed and Density, forward, frozen at its start or reverse as S09 sets, a grain at once. Several at once. The loop waits while any is held, a new recording ends them. At Density's centre: two grains at once, regular, the slice as it is. On the page, also a press on the waveform | |

| LED | |
|---|---|
| Glow | Grains falling, dimmer as the sand runs out |
| Flash | Recording |
| Pulse | At a stop: unison, speed 1×, 100 ms, Hann, filter off |
| Steady | P11 held: the twin last turned |

| To play | Set |
|---|---|
| Rhythm | Density below the centre, Shape up, Size short. Hold the slices: each its own pulse |
| Drums by hand | Density at the centre, Shape up: tap the slices, several at once |
| Cloud | Density at the top, Random position up, Size long, Reverb (P11 + S37) up |
| Stretch | Speed at ¼ or ½, Density up. S09 down to hear it backwards |
| A moment held | S09 at the centre, Density up, a little Random position. Speed scrubs the point |
| Shimmer | Random pitch (P11 + S33) up, S07 up: octaves |

## Play in the Browser

[francescodimaggio.nl/clessidra](https://francescodimaggio.nl/clessidra/): the same firmware, compiled to WebAssembly. A file you choose becomes the recording, up to 30 s, and plays on as the input. The demo, an 8 s phrase, plays in: a tap on Rec keeps it.

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
