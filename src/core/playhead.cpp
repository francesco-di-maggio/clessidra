#include "playhead.h"

#include <cmath>

namespace clessidra
{
void Playhead::Restart()
{
    direction_ = 1.0;
    stopped_   = false;
}

double Playhead::Step(double phase,
                      double inc,
                      End    end,
                      double low,
                      double high,
                      double margin)
{
    if(end == End::Loop)
    {
        direction_         = 1.0;
        stopped_           = false;
        const double next  = phase + inc;
        const double width = high - low;
        return next - width * std::floor((next - low) / width);
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
    if(forward ? next < high - margin : next >= low + margin)
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
