#include "layers.h"

#include <cmath>

namespace clessidra
{
void Layers::Add(size_t knob, float value)
{
    knobs_[knob].layered = true;
    knobs_[knob].second  = value;
}

void Layers::Process(const std::array<float, Controls::kNumKnobs>& knobs,
                     bool                                          second)
{
    if(second != second_)
    {
        for(auto& k : knobs_)
            k.caught = !k.layered;
        turned_ = Controls::kNumKnobs;
    }
    second_ = second;

    for(size_t i = 0; i < Controls::kNumKnobs; i++)
    {
        Knob&       k     = knobs_[i];
        const float now   = knobs[i];
        float&      value = second_ && k.layered ? k.second : k.first;
        // Power-up: every knob drives its first value at once
        if(!ready_)
            k.last = now;
        const bool crossed = (k.last - value) * (now - value) <= 0.0f;
        if(!k.caught && (std::fabs(now - value) < kPickUp || crossed))
            k.caught = true;
        if(k.caught)
        {
            if(second_ && k.layered && std::fabs(now - k.last) > 1e-4f)
                turned_ = i;
            value = now;
        }
        k.last = now;
    }
    ready_ = true;
}

} // namespace clessidra
