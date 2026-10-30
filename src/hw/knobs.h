#pragma once

#include <array>
#include <daisy_seed.h>
#include "controls.h"

namespace clessidra
{
// Knobs S30-S35 and faders S36 (left), S37 (right) on ADC A0-A7, read as 0..1.
class Knobs
{
  public:
    // update_rate: Process() calls per second
    void Init(daisy::DaisySeed& hw, float update_rate);
    void Process();

    float Value(size_t k) const { return knobs_[k].Value(); }

  private:
    std::array<daisy::AnalogControl, Controls::kNumKnobs> knobs_;
};

} // namespace clessidra
