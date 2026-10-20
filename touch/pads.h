#pragma once

#include <array>
#include "daisy_seed.h"

namespace clessidra {

// Capacitive pads P00-P11 on the MPR121 (I2C), with touch state and pressure.
class Pads {
public:
    static constexpr size_t kNumPads = 12;

    // Pressure 0..1 maps pad delta (baseline - filtered) from kDeltaFloor to kMaxDelta.
    // Measured on the Touch with TouchMIDI: light touch 50-250, hard press 590-670.
    static constexpr int32_t kDeltaFloor = 5;
    static constexpr int32_t kMaxDelta   = 500;

    void Init();

    // Reads every pad over I2C, which blocks: call from the main loop.
    void Process();

    bool IsTouched(size_t p) const { return (_touched & (1 << p)) != 0; }
    bool AnyTouched() const { return _touched != 0; }
    float Pressure(size_t p) const { return _pressure[p]; }

private:
    daisy::Mpr121I2C _mpr;
    uint16_t _touched = 0;
    std::array<float, kNumPads> _pressure{};
};

} // namespace clessidra
