#pragma once

#include "daisy_seed.h"
#include "../touch/touch.h"
#include "../grain/engine.h"

namespace clessidra {

// Maps the Touch controls to the engine and drives the LED. Holding P11 switches
// S30, S31, S32, S34 and S37 to a second layer.
class ClessidraUI {
public:
    ClessidraUI(Touch& touch, Engine& engine) : _touch{ touch }, _engine{ engine } {}

    // Main loop, after the pads are read.
    void Process(daisy::DaisySeed& hw);

private:
    // A value set by a knob shared between layers. After a layer change the knob
    // moves the value from where it is, scaled so both reach the end stops together:
    // no jump, no dead zone.
    class Takeover {
    public:
        explicit Takeover(float value) : _value{ value }, _previous{ value } {}

        void Engage(float knob) { _previous = _origin = knob; }
        void Process(float knob);
        float Value() const { return _value; }

    private:
        static constexpr float kDeadband = 0.02f; // around the knob at Engage(): noise

        float _value;
        float _previous;
        float _origin = -1.0f; // knob at Engage() until it moves past the deadband
    };

    struct Layer {
        Takeover s30, s31, s32, s34, s37;

        void Engage(const Knobs& k);
        void Process(const Knobs& k);
    };

    void _led(daisy::DaisySeed& hw);

    Touch& _touch;
    Engine& _engine;
    Layer _main{ Takeover(0.0f), Takeover(0.0f), Takeover(0.0f), Takeover(0.0f), Takeover(0.0f) };
    // Mix, window shape, spread, chance, gain: the engine defaults
    Layer _shift{ Takeover(1.0f), Takeover(0.5f), Takeover(0.5f), Takeover(1.0f), Takeover(1.0f) };
    bool _started = false;
    bool _shifted = false;
    uint32_t _grain_starts = 0;
    uint32_t _blink_start = 0;
};

} // namespace clessidra
