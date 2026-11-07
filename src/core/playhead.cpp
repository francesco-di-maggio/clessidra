#include "playhead.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
namespace
{
    // The last phase inside the span
    const double kLast = std::nextafter(1.0, 0.0);
} // namespace

void Playhead::Restart(bool backwards)
{
    phase_   = backwards ? kLast : 0.0;
    forward_ = !backwards;
    ended_   = false;
    wrapped_ = false;
}

void Playhead::Advance(double step, End end)
{
    wrapped_ = false;
    if(step != 0.0)
    {
        if(ended_ && (step > 0.0) == forward_)
            return;
        forward_ = step > 0.0;
        ended_   = false;
    }
    phase_ += step;
    if(phase_ >= 0.0 && phase_ < 1.0)
        return;

    if(end == End::Loop)
    {
        phase_ -= std::floor(phase_);
        wrapped_ = true;
    }
    else
        ended_ = true;
    phase_ = std::clamp(phase_, 0.0, kLast);
}

double Playhead::Left() const
{
    if(ended_)
        return 0.0;
    return forward_ ? 1.0 - phase_ : phase_;
}

} // namespace clessidra
