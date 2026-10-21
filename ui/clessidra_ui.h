#pragma once

#include <array>
#include "daisy_seed.h"
#include "../touch/touch.h"
#include "../grain/engine.h"

namespace clessidra {

// Maps the Touch controls to the engine and drives the LED. Holding P11 switches
// the knobs to a second layer.
class ClessidraUI {
public:
    ClessidraUI(Touch& touch, Engine& engine);

    // Main loop, after the pads are read.
    void Process(daisy::DaisySeed& hw);

private:
    // A value set by a knob shared between layers. After a layer change the knob
    // moves the value from where it is, scaled so both reach the end stops together:
    // no jump, no dead zone.
    class Takeover {
    public:
        explicit Takeover(float value = 0.0f) : _value{ value }, _previous{ value } {}

        void Engage(float knob) { _previous = _origin = knob; }
        void Process(float knob);
        float Value() const { return _value; }

    private:
        static constexpr float kDeadband = 0.02f; // around the knob at Engage(): noise

        float _value;
        float _previous;
        float _origin = -1.0f; // knob at Engage() until it moves past the deadband
    };

    // A value per knob
    struct Layer {
        std::array<Takeover, Knobs::kNumKnobs> values;

        float operator[](size_t knob) const { return values[knob].Value(); }
        void Engage(const Knobs& k);
        void Process(const Knobs& k);
    };

    void _led(daisy::DaisySeed& hw);

    Touch& _touch;
    Engine& _engine;
    Layer _main;
    Layer _shift;
    bool _started = false;
    bool _shifted = false;
    bool _frozen = false;
    float _speed = 0.5f; // S36 at the freeze
    uint32_t _grain_starts = 0;
    uint32_t _blink_start = 0;
};

} // namespace clessidra
