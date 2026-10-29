#pragma once

#include "daisy_seed.h"
#include "controls.h"
#include "nocopy.h"
#include "knobs.h"
#include "pads.h"
#include "switches.h"

namespace clessidra {

// The Simple Touch: the Seed, its LED and the panel, read with libDaisy's own drivers.
class Touch {
public:
    Touch() = default;

    void Init(daisy::SaiHandle::Config::SampleRate sample_rate, size_t block_size) {
        _seed.Init(true);
        _seed.SetAudioSampleRate(sample_rate);
        _seed.SetAudioBlockSize(block_size);
        // Knobs are smoothed at the callback rate, set above
        _knobs.Init(_seed, _seed.AudioCallbackRate());
        _seed.adc.Start();
        _switches.Init();
        _pads.Init();
        // TIM3 channel 2 drives PC7, the Seed's LED. 16-bit period: about 3.7 kHz.
        _led.Init(daisy::PWMHandle::Config(daisy::PWMHandle::Config::Peripheral::TIM_3));
        _led.Channel2().Init();
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

    // 0..1, duty cycle
    void SetLed(float value) { _led.Channel2().Set(value); }

    daisy::DaisySeed& seed() { return _seed; }
    Knobs& knobs() { return _knobs; }
    Pads& pads() { return _pads; }
    Switches& switches() { return _switches; }

private:
    NOCOPY(Touch)
    daisy::DaisySeed _seed;
    daisy::PWMHandle _led;
    Knobs _knobs;
    Pads _pads;
    Switches _switches;
};

} // namespace clessidra
