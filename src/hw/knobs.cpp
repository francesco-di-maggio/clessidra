#include "knobs.h"

using namespace daisy;
using namespace daisy::seed;

namespace clessidra {

void Knobs::Init(DaisySeed& hw, float update_rate) {
    const Pin pins[kNumKnobs] = { A0, A1, A2, A3, A4, A5, A6, A7 };
    AdcChannelConfig cfg[kNumKnobs];
    for (size_t k = 0; k < kNumKnobs; k++) {
        cfg[k].InitSingle(pins[k]);
    }
    hw.adc.Init(cfg, kNumKnobs);
    for (size_t k = 0; k < kNumKnobs; k++) {
        _knobs[k].Init(hw.adc.GetPtr(k), update_rate);
    }
}

void Knobs::Process() {
    for (auto& k : _knobs) {
        k.Process();
    }
}

} // namespace clessidra
