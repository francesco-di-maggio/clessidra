#pragma once

#include <array>
#include <daisy_seed.h>

namespace clessidra
{
// Capacitive pads P00-P11 on the MPR121 (I2C), with touch state and pressure.
class Pads
{
  public:
    static constexpr size_t kNumPads = 12;

    // Delta counts (baseline - filtered). Measured: a press 200-590, cross-talk
    // on a neighbouring pad up to 9, release tails up to 13.
    static constexpr uint8_t kTouchThreshold   = 40;
    static constexpr uint8_t kReleaseThreshold = 20;

    // Pressure 0..1 maps the delta from kReleaseThreshold to kMaxDelta.
    // Measured: light 200-330, medium 360-440, hard 470-590.
    static constexpr int32_t kMaxDelta  = 600;
    static constexpr float   kSmoothing = 0.4f; // per Process() call
    // released pressure below this is 0
    static constexpr float kPressureFloor = 0.01f;

    void Init();

    // Reads every pad over I2C, which blocks: call from the main loop.
    void Process();

    bool  IsTouched(size_t p) const { return (touched_ & (1 << p)) != 0; }
    float Pressure(size_t p) const { return pressure_[p]; }

  private:
    void Configure();

    daisy::Mpr121I2C            mpr_;
    uint16_t                    touched_ = 0;
    std::array<float, kNumPads> pressure_{};
};

} // namespace clessidra
