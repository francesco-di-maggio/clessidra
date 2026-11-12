# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

A granular sampler. Record a sound, then play it as grains: Speed moves through it, Density sets how they fall. The input is always captured, so a tap on Rec keeps the moment that just passed. Two layers: each knob alone, and its twin with P11 held. After a layer change a knob moves its value from where it is, so nothing jumps.

| Control | Alone | P11 held |
|---|---|---|
| S36 | Speed: 0 to 4×, stops at ¼, ½, 1, 2 and 4×, the pitch kept. Frozen: scrubs through the loop, a full stroke the whole loop, the speed taken up again after | Reverse: how many grains read the other way, none to all. The phrase keeps its direction, each grain turns |
| S37 | Density: grains per second, 0.25 to 320, whatever their size. None at the bottom. 32 voices: all sound at the top with Size at its centre, and a new grain is dropped while they do | Volume: the output, off to 0 dB |
| S30 | Position: where the loop starts, anywhere in the recording. The playhead keeps its place in the loop, so it moves with it, playing or frozen | Filter: low-pass to 200 Hz, off at the centre, high-pass to 2 kHz |
| S35 | Length: a share of what is left after Position, at the top to the recording's end | Reverb: none to most, decay 0.5 to 16 s |
| S31 | Size: 5 ms to 2 s, 100 ms at the centre, at most half the recording | Random size: each grain part of the way to a random size, up to about an octave either way at the centre |
| S32 | Spray: grains stray from the playhead, evenly, up to the whole loop, across the stereo field with it | Random timing: when grains start, from a regular pulse to random, as many per second |
| S33 | Pitch: ±24 semitones, stops at 0, 1, 3, 4, 7, 12, 24 | Random pitch: off at the centre. Up: free, to ±12 semitones. Down: in octaves, up to two grains in three an octave up or down |
| S34 | Shape: the grain's window, square, decay, bell, reverse at even steps along the knob, and everything between. Square: at full level its whole length. Decay: at once up, then falling. Bell: smooth both ways. Reverse: rising to its end | Random shape: each grain a different window, anywhere from square to reverse |
| S09 | Direction: up forward, centre freeze, down reverse | |
| S07 | End, at the loop's end: down once and no more, centre turned over to run back, up round again | |

| Pad | Alone | P11 held |
|---|---|---|
| P10 | Record. The input is always captured. Tap: the last 8 s. Hold: up to 30 s, the grains playing on. Silence at the ends is trimmed, the new recording pours in over 2 s (the first rises from silence), its slices are found | Listen: the input heard, nothing recorded |
| P11 | Shift, held | |
| P00 | Stop, as a tape machine: held, everything slows to a halt over 0.6 s, the pitch falling. Released: back to speed over 0.6 s | |
| P01 | Reset: the twins back to their defaults | |
| P02 | Trigger: playback again from the loop's start (its end in reverse), with a grain at once. After a loop played once, it pours again | |
| P03–P09 | Slices: the loop in seven, its start then the recording's strongest attacks inside it (even parts without attacks), left to right on the board (P03, P04, P08, P05, P09, P06, P07). Held: plays its slice from the start, round it, at Speed and Density, forward, frozen at its start or reverse as S09 sets, to its end as S07 sets (once and no more, turned over, or round it), a grain at once. Several at once. The loop waits while any is held, a new recording ends them. At Density's bottom: two grains at once, regular, the slice as it is | |

| LED | |
|---|---|
| Glow | Grains falling |
| Flash | Recording |
| Pulse | At a stop: unison, speed 1×, 100 ms, filter off, random pitch off |
| Steady | P11 held: the last knob turned |

| To play | Set |
|---|---|
| Rhythm | Density low, Random timing (P11 + S32) down, Shape on decay, Size short. Size sets how long each hit lasts. |
| Drops | Density near the bottom, Size short: single grains, now and then. |
| Drums by hand | Density at the bottom, Shape on square: tap the slices, several at once. |
| Cloud | Density at the top, Size 150 to 300 ms, Shape on bell, Spray up, Random timing (P11 + S32) full, Reverb (P11 + S35) up. |
| Stretch | Speed at ¼ or ½, Density up. S09 down to hear it backwards. |
| Frozen | S09 at the centre, Density at the top, Size long, a little Spray, Reverb up. Speed scrubs the point. |
| One pour | S07 down: the loop plays once and stops. Trigger pours it again. |
| Shimmer | Random pitch (P11 + S33) below the centre: octaves. |
| Backwards syllables | Reverse (P11 + S36) at the top: the phrase in order, each grain backwards. |

## Play in the Browser

[francescodimaggio.nl/clessidra](https://francescodimaggio.nl/clessidra/): the same firmware, compiled to WebAssembly. A file you choose becomes the recording, up to 30 s, and plays on as the input. The demo, an 8 s phrase, plays in: a tap on Rec keeps it.

## Install

Download the [binary](https://github.com/francesco-di-maggio/clessidra/releases/latest/download/clessidra.bin) and flash it with the [Daisy Seed web programmer](https://flash.daisy.audio/).

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
