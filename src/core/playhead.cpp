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
    heading_ = 1;
    forward_ = !backwards;
    ended_   = false;
}

void Playhead::Jump(double phase)
{
    phase_ = std::clamp(phase, 0.0, kLast);
    ended_ = false;
}

void Playhead::Advance(double step, End end)
{
    if(end != End::Turn)
        heading_ = 1;
    const double move = step * heading_;
    if(move != 0.0)
    {
        if(ended_ && end == End::Once && (move > 0.0) == forward_)
            return;
        forward_ = move > 0.0;
        ended_   = false;
    }
    phase_ += move;
    if(phase_ >= 0.0 && phase_ < 1.0)
        return;

    const bool past_end = phase_ >= 1.0;
    switch(end)
    {
        case End::Loop: phase_ -= std::floor(phase_); break;
        case End::Turn:
            phase_   = past_end ? 2.0 - phase_ : -phase_;
            heading_ = -heading_;
            forward_ = !past_end;
            break;
        case End::Once: ended_ = true; break;
    }
    phase_ = std::clamp(phase_, 0.0, kLast);
}

double Playhead::Left() const
{
    if(ended_)
        return 0.0;
    return forward_ ? 1.0 - phase_ : phase_;
}

} // namespace clessidra
