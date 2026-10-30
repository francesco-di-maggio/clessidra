#include "knobs.h"

namespace clessidra
{
void Knobs::Init(daisy::DaisySeed& hw, float update_rate)
{
    using namespace daisy::seed;
    const daisy::Pin        pins[kNumKnobs] = {A0, A1, A2, A3, A4, A5, A6, A7};
    daisy::AdcChannelConfig cfg[kNumKnobs];
    for(size_t k = 0; k < kNumKnobs; k++)
    {
        cfg[k].InitSingle(pins[k]);
    }
    hw.adc.Init(cfg, kNumKnobs);
    for(size_t k = 0; k < kNumKnobs; k++)
    {
        knobs_[k].Init(hw.adc.GetPtr(k), update_rate);
    }
}

void Knobs::Process()
{
    for(auto& k : knobs_)
    {
        k.Process();
    }
}

} // namespace clessidra
