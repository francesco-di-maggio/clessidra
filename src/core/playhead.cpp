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
    phase_ = backwards ? kLast : 0.0;
}

void Playhead::Jump(double phase)
{
    phase_ = std::clamp(phase, 0.0, kLast);
}

void Playhead::Advance(double step)
{
    phase_ += step;
    if(phase_ >= 0.0 && phase_ < 1.0)
        return;
    phase_ -= std::floor(phase_);
    phase_ = std::min(phase_, kLast);
}

} // namespace clessidra
