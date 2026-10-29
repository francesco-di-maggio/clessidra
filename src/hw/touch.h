#pragma once

#include "daisy_seed.h"
#include "controls.h"
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

    // The panel as the UI reads it
    Controls Read() const {
        Controls c;
        for (size_t k = 0; k < Controls::kNumKnobs; k++) c.knobs[k] = _knobs.Value(k);
        c.switches[Controls::S09] = Controls::Position(_switches.Position(Switches::S09));
        c.switches[Controls::S07] = Controls::Position(_switches.Position(Switches::S07));
        for (size_t p = 0; p < Controls::kNumPads; p++) {
            c.touched[p] = _pads.IsTouched(p);
            c.pressure[p] = _pads.Pressure(p);
        }
        c.now_ms = daisy::System::GetNow();
        return c;
    }

    Knobs& knobs() { return _knobs; }
    Pads& pads() { return _pads; }
    Switches& switches() { return _switches; }

private:
    Knobs _knobs;
    Pads _pads;
    Switches _switches;
};

} // namespace clessidra
