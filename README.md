# Clessidra

A Simple Touch instrument inspired by the hourglass, in progress.

## Current Status

Sound and mappings are explored in Max, with the Touch running [TouchMIDI](https://github.com/francesco-di-maggio/TouchMIDI) as a USB-MIDI controller. Once the instrument starts to feel right, it moves to C++ firmware running on the Touch itself.

## Instruments

| Instrument | Platform | Status | Folder |
|---|---|---|---|
| Granulator | Max | In Progress | [`max/patchers/granulator`](max/patchers/granulator) |

## Max

- `max/patchers/`: one folder per instrument, each with its own README.
- `max/javascript/`: soft-takeover modes (pickup, scale, glide) shared by the patches.
- `max/examples/pickupmodes.maxpat`: demo of the soft-takeover modes.

## License

MIT
