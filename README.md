# Clessidra

Granular instrument for the Synthux Simple Touch, inspired by the hourglass.

[Play it in the browser](https://francesco-di-maggio.github.io/clessidra/)

## Controls

<img src="touch.jpeg" width="300"/>

| Control | Function |
|---|---|
| S30 / S35 | Selection start / end |
| S31 | Grain size, 20 ms – 2 s |
| S32 | Random position |
| S33 | Pitch, ±2 octaves |
| S34 | Random pitch, ±2 octaves |
| S36 | Speed, 0–4. Scrubs while frozen. |
| S37 | Flow: off, 0.1–16 grains at once |
| S09 | Forward / freeze / reverse |
| S07 | Grain starts: periodic / half random / random |
| P00, P02–P09 | Places in the selection. Pressure sets the flow. |
| P01 | Hold: stop the flow |
| P10 | Tap: keep the last 10 s. Hold: record. |
| P11 | Hold: second layer |

Second layer:

| Control | Function |
|---|---|
| S31 | Window shape |
| S32 | Stereo spread |
| S34 | Reverb |
| S35 | Tone: lowpass / off / highpass |
| S36 | Dry / wet |
| S37 | Resample: what P10 records, input to grains |

| LED | |
|---|---|
| Glow | Grains playing |
| Pulse | A control at its centre |
| Flashing | P10 touched |
| Steady | P11 held: brightness shows the value |

## Install

Download the [binary](https://github.com/francesco-di-maggio/clessidra/releases/latest/download/clessidra.bin) and flash it with the [Daisy Seed web programmer](https://electro-smith.github.io/Programmer/).

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
