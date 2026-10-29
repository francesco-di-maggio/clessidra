#pragma once

namespace clessidra {

// A value set by a knob shared between layers. After a layer change the knob
// moves the value from where it is, scaled so both reach the end stops together:
// no jump, no dead zone.
class Takeover {
public:
    explicit Takeover(float value = 0.0f) : _value{ value }, _previous{ value } {}

    void engage(float knob) { _previous = _origin = knob; }
    void process(float knob);
    float value() const { return _value; }

private:
    static constexpr float kDeadband = 0.02f; // around the knob at engage(): noise

    float _value;
    float _previous;
    float _origin = -1.0f; // knob at engage() until it moves past the deadband
};

} // namespace clessidra
