#pragma once

#include <array>
#include <daisy_seed.h>

namespace clessidra
{
// Knobs S30-S35 and faders S36 (left), S37 (right) on ADC A0-A7, read as 0..1.
class Knobs
{
  public:
    static constexpr size_t kNumKnobs = 8;

    enum Knob : size_t
    {
        S30,
        S31,
        S32,
        S33,
        S34,
        S35,
        S36,
        S37
    };

    // update_rate: Process() calls per second
    void Init(daisy::DaisySeed& hw, float update_rate);
    void Process();

    float Value(size_t k) const { return knobs_[k].Value(); }

  private:
    std::array<daisy::AnalogControl, kNumKnobs> knobs_;
};

} // namespace clessidra
