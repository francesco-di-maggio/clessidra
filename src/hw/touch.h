#pragma once

#include "daisy_seed.h"
#include "knobs.h"
#include "pads.h"
#include "switches.h"

namespace clessidra {

// Simple Touch controls, read with libDaisy's own drivers.
class Touch {
public:
    // Call after the audio block size is set: knobs are smoothed at the callback rate.
    void Init(daisy::DaisySeed& hw) {
        _knobs.Init(hw, hw.AudioCallbackRate());
        hw.adc.Start();
        _switches.Init();
        _pads.Init();
    }

    // Audio callback, once per block.
    void ProcessControls() {
        _knobs.Process();
        _switches.Process();
    }

    // Main loop.
    void ProcessPads() { _pads.Process(); }

    Knobs& knobs() { return _knobs; }
    Pads& pads() { return _pads; }
    Switches& switches() { return _switches; }

private:
    Knobs _knobs;
    Pads _pads;
    Switches _switches;
};

} // namespace clessidra
