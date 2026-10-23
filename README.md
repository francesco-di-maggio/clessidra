# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

[Play it in the browser](https://francesco-di-maggio.github.io/clessidra/)

## Controls

<img src="touch.jpeg" width="300"/>

| Control | Function |
|---|---|
| S30 / S35 | Selection start / end |
| S31 | Grain size, 10 ms – half the recording (4 s at 8 s) |
| S32 | Random position |
| S33 | Pitch, ±2 octaves |
| S34 | Random pitch, ±2 octaves |
| S36 | Speed, 0–4. Scrubs while frozen. |
| S37 | Density: off, 0.1–16 grains at once |
| S09 | Forward / freeze / reverse |
| S07 | Grain starts: periodic / half random / random |
| P03–P09 | Places in the selection, at the S37 density (8 with S37 off). Pressure crossfades from the playhead to the place. |
| P02 | Hold: stop the grains |
| P01 | Tap: the sound back to the defaults, knobs take over from there. Levels kept. |
| P00 | Hold: levels |
| P10 | Tap: keep the last 8 s. Hold: record. |
| P11 | Hold: second layer |

Second layer:

| Control | Function |
|---|---|
| S30 | Reverse: share of grains read backwards |
| S31 | Shape: reverse percussive / Hann / percussive |
| S32 | Size variation: each grain from half to twice the size |
| S33 | Reverb |
| S34 | Harmonic: share of random pitches on partials |
| S35 | Tone: lowpass 200 Hz / off / highpass 2 kHz |
| S36 | Dry / wet |
| S37 | Resample: what P10 records, input to grains |

Levels:

| Control | Function |
|---|---|
| S36 | Input level, centre unity, up to +12 dB |
| S37 | Output level |

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
