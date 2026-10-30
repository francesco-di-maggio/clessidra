#pragma once

#include <cmath>
#include <Utility/dsp.h>

namespace clessidra
{
// State variable filter, trapezoidal integration (zero-delay feedback), after
// Andrew Simper, "Linear Trapezoidal Integrated State Variable Filter"
// (Cytomic, 2013). Linear, cutoff exact up to Nyquist, stable while the
// settings change.
class Svf
{
  public:
    void Set(float hz, float q, float sample_rate)
    {
        const float g = std::tan(PI_F * hz / sample_rate);
        k_            = 1.0f / q;
        a1_           = 1.0f / (1.0f + g * (g + k_));
        a2_           = g * a1_;
        a3_           = g * a2_;
    }

    void Process(float x)
    {
        const float v3 = x - ic2_;
        const float v1 = a1_ * ic1_ + a2_ * v3;
        const float v2 = ic2_ + a2_ * ic1_ + a3_ * v3;
        ic1_           = 2.0f * v1 - ic1_;
        ic2_           = 2.0f * v2 - ic2_;
        low_           = v2;
        high_          = x - k_ * v1 - v2;
    }

    float Low() const { return low_; }
    float High() const { return high_; }

  private:
    float k_ = 1.0f, a1_ = 0.0f, a2_ = 0.0f, a3_ = 0.0f;
    float ic1_ = 0.0f, ic2_ = 0.0f;
    float low_ = 0.0f, high_ = 0.0f;
};

} // namespace clessidra
