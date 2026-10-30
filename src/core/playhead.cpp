#include "playhead.h"

#include <cmath>

namespace clessidra
{
void Playhead::Restart()
{
    direction_ = 1.0;
    stopped_   = false;
}

double Playhead::Step(double phase, double inc, End end, double margin)
{
    if(end == End::Loop)
    {
        direction_        = 1.0;
        stopped_          = false;
        const double next = phase + inc;
        return next - std::floor(next);
    }

    const double moving = inc * direction_;
    if(stopped_)
    {
        // Flipped, moving back into the span: it falls again
        const bool inward = at_top_ ? moving < 0.0 : moving > 0.0;
        if(end == End::OneShot && !inward)
            return phase;
        stopped_ = false;
    }

    const bool   forward = moving >= 0.0;
    const double next    = phase + moving;
    if(forward ? next < 1.0 - margin : next >= margin)
        return next;

    if(end == End::PingPong)
    {
        direction_ = -direction_;
        return phase;
    }
    stopped_ = true;
    at_top_  = forward;
    return phase;
}

} // namespace clessidra
