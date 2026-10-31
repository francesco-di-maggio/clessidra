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
                      double margin,
                      double drift)
{
    if(end == End::Loop)
    {
        direction_          = 1.0;
        stopped_            = false;
        const double moving = inc - drift;
        if(moving == 0.0)
            return phase; // keeping up with the span
        const double next  = phase + moving;
        const double width = high - low;
        if(width <= 0.0)
            return low; // a NaN here would stay in the reverb
        return next - width * std::floor((next - low) / width);
    }

    const double moving = inc * direction_ - drift;
    if(stopped_)
    {
        // Flipped, moving back into the span: it falls again
        const bool inward = at_top_ ? moving < 0.0 : moving > 0.0;
        if(end == End::OneShot && !inward)
            return phase;
        stopped_ = false;
    }

    if(moving == 0.0)
        return phase; // keeping up with the span

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
