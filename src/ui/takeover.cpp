#include "takeover.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
void Takeover::Process(float knob)
{
    float from = previous_;
    previous_  = knob;
    if(origin_ >= 0.0f)
    {
        if(std::fabs(knob - origin_) < kDeadband)
            return;
        // The motion inside the deadband counts
        from    = origin_;
        origin_ = -1.0f;
    }
    Move(from, knob);
}

void Takeover::Move(float from, float to)
{
    if(centred_ && (from - 0.5f) * (to - 0.5f) < 0.0f)
    {
        Move(from, 0.5f);
        from = 0.5f;
    }
    const float delta = to - from;
    if(delta == 0.0f)
        return;
    // Towards the next stop, the value covers its remaining range while the
    // knob covers its own
    const bool up = delta > 0.0f;
    const bool half
        = centred_
          && (up ? from < 0.5f && value_ < 0.5f : from > 0.5f && value_ > 0.5f);
    const float stop  = half ? 0.5f : up ? 1.0f : 0.0f;
    const float edge  = up ? stop + kMargin : stop - kMargin;
    const float scale = (edge - value_) / (edge - from);
    value_            = std::clamp(
        value_ + std::clamp(scale, 0.1f, 10.0f) * delta, 0.0f, 1.0f);
    // At the stop with the knob: there exactly, from then on as one
    if(to == stop)
        value_ = stop;
}

} // namespace clessidra
