#include "takeover.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

void Takeover::Process(float knob) {
    float delta = knob - _previous;
    _previous = knob;
    if (_origin >= 0.0f) {
        if (std::fabs(knob - _origin) < kDeadband) return;
        // The motion inside the deadband counts
        delta = knob - _origin;
        _origin = -1.0f;
    }
    // Towards an end stop, the value covers its remaining range while the knob covers its own
    const float scale = delta > 0.0f ? (1.001f - _value) / (1.001f - (knob - delta))
                                     : (0.001f + _value) / (0.001f + (knob - delta));
    _value = std::clamp(_value + std::clamp(scale, 0.1f, 10.0f) * delta, 0.0f, 1.0f);
}

} // namespace clessidra
