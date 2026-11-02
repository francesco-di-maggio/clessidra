#include "layers.h"

#include <cmath>

namespace clessidra
{
void Layers::Add(size_t layer, size_t knob, float value)
{
    knobs_[knob].used[layer]  = true;
    knobs_[knob].value[layer] = value;
}

void Layers::Process(const std::array<float, Controls::kNumKnobs>& knobs,
                     size_t                                        layer)
{
    if(layer != layer_)
    {
        // A knob with no function on the new layer keeps its own
        for(auto& k : knobs_)
            k.caught = !k.used[layer];
        turned_ = Controls::kNumKnobs;
    }
    layer_ = layer;

    for(size_t i = 0; i < Controls::kNumKnobs; i++)
    {
        Knob&       k     = knobs_[i];
        const float now   = knobs[i];
        const bool  own   = !k.used[layer_];
        float&      value = k.value[own ? 0 : layer_];
        // Power-up: every knob drives its own value at once
        if(!ready_)
            k.last = now;
        const bool crossed = (k.last - value) * (now - value) <= 0.0f;
        if(!k.caught && (std::fabs(now - value) < kPickUp || crossed))
            k.caught = true;
        if(k.caught)
        {
            if(layer_ > 0 && !own && std::fabs(now - k.last) > 1e-4f)
                turned_ = i;
            value = now;
        }
        k.last = now;
    }
    ready_ = true;
}

} // namespace clessidra
