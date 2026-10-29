#pragma once

#include <array>
#include "daisy_seed.h"

namespace clessidra {

// Capacitive pads P00-P11 on the MPR121 (I2C), with touch state and pressure.
class Pads {
public:
    static constexpr size_t kNumPads = 12;

    // Delta counts (baseline - filtered). Measured: a press 200-590, cross-talk on a
    // neighbouring pad up to 9, release tails up to 13.
    static constexpr uint8_t kTouchThreshold   = 40;
    static constexpr uint8_t kReleaseThreshold = 20;

    // Pressure 0..1 maps the delta from kReleaseThreshold to kMaxDelta.
    // Measured: light 200-330, medium 360-440, hard 470-590.
    static constexpr int32_t kMaxDelta = 600;
    static constexpr float kSmoothing     = 0.4f;  // per process() call
    static constexpr float kPressureFloor = 0.01f; // released pressure below this is 0

    void init();

    // Reads every pad over I2C, which blocks: call from the main loop.
    void process();

    bool is_touched(size_t p) const { return (_touched & (1 << p)) != 0; }
    float pressure(size_t p) const { return _pressure[p]; }

private:
    void _configure();

    daisy::Mpr121I2C _mpr;
    uint16_t _touched = 0;
    std::array<float, kNumPads> _pressure{};
};

} // namespace clessidra
