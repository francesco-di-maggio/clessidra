#include "pads.h"

#include <algorithm>

using namespace daisy;

namespace clessidra {

void Pads::Init() {
    Mpr121I2C::Config cfg;
    _mpr.Init(cfg);
}

void Pads::Process() {
    _touched = _mpr.Touched();
    for (size_t p = 0; p < kNumPads; p++) {
        if (!IsTouched(p)) {
            _pressure[p] = 0.0f;
            continue;
        }
        const int32_t delta = int32_t(_mpr.BaselineData(p)) - int32_t(_mpr.FilteredData(p));
        const float norm = float(delta - kDeltaFloor) / float(kMaxDelta - kDeltaFloor);
        _pressure[p] = std::clamp(norm, 0.0f, 1.0f);
    }
}

} // namespace clessidra
