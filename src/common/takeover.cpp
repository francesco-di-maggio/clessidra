#include "takeover.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
void Takeover::Process(float knob)
{
    float delta = knob - previous_;
    previous_   = knob;
    if(origin_ >= 0.0f)
    {
        if(std::fabs(knob - origin_) < kDeadband)
            return;
        // The motion inside the deadband counts
        delta   = knob - origin_;
        origin_ = -1.0f;
    }
    // Towards an end stop, the value covers its remaining range while the knob
    // covers its own
    const float scale = delta > 0.0f
                            ? (1.001f - value_) / (1.001f - (knob - delta))
                            : (0.001f + value_) / (0.001f + (knob - delta));
    value_ = std::clamp(
        value_ + std::clamp(scale, 0.1f, 10.0f) * delta, 0.0f, 1.0f);
}

} // namespace clessidra
