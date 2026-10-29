#pragma once

#include <cmath>

namespace clessidra {

// State variable filter, trapezoidal integration (zero-delay feedback), after Andrew
// Simper, "Linear Trapezoidal Integrated State Variable Filter" (Cytomic, 2013).
// Linear, cutoff exact up to Nyquist, stable while the settings change.
class Svf {
public:
    void set(float hz, float q, float sample_rate) {
        const float g = std::tan(float(M_PI) * hz / sample_rate);
        _k = 1.0f / q;
        _a1 = 1.0f / (1.0f + g * (g + _k));
        _a2 = g * _a1;
        _a3 = g * _a2;
    }

    void process(float x) {
        const float v3 = x - _ic2;
        const float v1 = _a1 * _ic1 + _a2 * v3;
        const float v2 = _ic2 + _a2 * _ic1 + _a3 * v3;
        _ic1 = 2.0f * v1 - _ic1;
        _ic2 = 2.0f * v2 - _ic2;
        _low = v2;
        _high = x - _k * v1 - v2;
    }

    float low() const { return _low; }
    float high() const { return _high; }

private:
    float _k = 1.0f, _a1 = 0.0f, _a2 = 0.0f, _a3 = 0.0f;
    float _ic1 = 0.0f, _ic2 = 0.0f;
    float _low = 0.0f, _high = 0.0f;
};

} // namespace clessidra
