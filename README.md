# Clessidra

A Simple Touch instrument inspired by the hourglass, in progress.

## Setup

1. Flash the Touch with [TouchMIDI](https://github.com/francesco-di-maggio/TouchMIDI).
2. Open `max/patchers/granulator/granulator.maxpat` and `max/patchers/granulator/TouchGrain.maxpat` in Max.

## Max Patches

- `patchers/granulator/granulator.maxpat`: granular synth, with `p_grain` as the per-voice abstraction. Presets in `granulator.json`.
- `patchers/granulator/TouchGrain.maxpat`: reads TouchMIDI and drives the granulator.
- `examples/pickupmodes.maxpat`: demo of the soft-takeover modes (pickup, scale, glide) in `javascript/`.

The granulator started from Nobuyasu Sakonda's [sugarSynth](http://formantbros.jp/sako/download.html).

## License

MIT
