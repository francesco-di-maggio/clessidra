# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

## Controls

<img src="touch.jpeg" width="300"/>

| Control | Function |
|---|---|
| S36 | Speed: centre stop, up forward, down backward, up to 4 times. Speed 1 half way |
| S37 | Density: off, 0.3–130 grains per second |
| S30 | Position of the selection in the recording |
| S35 | Length of the selection |
| S31 | Grain size, 10 ms – half the recording, up to 4 s |
| S32 | Scatter: from periodic and plain to random starts, positions and sizes. Above the centre, random pitch |
| S33 | Pitch, ±2 octaves, stops at 0, 0.1, 1, 3, 4, 7, 12 and 24 semitones |
| S34 | Shape: reverse percussive / Hann / percussive |
| S07 | End: one-shot / ping-pong / loop |
| P03–P09 | Slices at the sound's onsets. A touch plays one, a firm press adds density |
| P10 | Tap: keep the last 8 s. Hold: record, up to 60 s |
| P11 | Hold: effects |

Effects, P11 held:

| Control | Function |
|---|---|
| S36 | Input level, centre unity, up to +12 dB |
| S37 | Output level |
| S35 | Dry / wet |
| S32 | Reverb |
| S33 | Resample: what P10 records, input to grains |
| S34 | Tone: lowpass 200 Hz / off / highpass 2 kHz |

| LED | |
|---|---|
| Glow | Grains playing |
| Pulse | A control at or turning within its centre |
| Flashing | P10 touched |
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
