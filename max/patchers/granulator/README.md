# Granulator

Granular sampler played from the Simple Touch.

## Files

- `granulator.maxpat`: the instrument. Runs 8 voices of `p_grain` in a `poly~`.
- `p_grain.maxpat`: per-voice grain abstraction.
- `granulator.json`: presets for `pattrstorage granulator`.
- `TouchGrain.maxpat`: the TouchMIDI template with granulator labels. Connects to the Touch and forwards its controls.

## Setup

1. Flash the Touch with [TouchMIDI](https://github.com/francesco-di-maggio/TouchMIDI).
2. Open `granulator.maxpat` and `TouchGrain.maxpat`.

The default sample is `rainstick.aif`, which ships with Max.

## Mapping

| Control | Parameter |
|---|---|
| S30 | playback start position |
| S31 | grain size |
| S32 | random position |
| S33 | pitch |
| S34 | random pitch |
| S35 | playback end position |
| S09 (left switch) | playback mode: reverse / freeze / forward |
| S36 (left fader) | playback speed (reverse, forward); playback position (freeze) |
| S37 (right fader) | gain |
| S07 (right switch) | mute / unmute / fuzz |
| P01 | load default sample |
| P10 | record |
| P11 | randomize |
| P00, P02-P09 | first press stores the current sound as preset 1; pressure interpolates through presets 1-8 |

## Credits

Based on Nobuyasu Sakonda's [sugarSynth](http://formantbros.jp/sako/download.html).
