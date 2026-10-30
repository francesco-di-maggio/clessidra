#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <Utility/delayline.h>

namespace clessidra
{
// Stereo-linked look-ahead peak limiter. The gain is the minimum over the
// coming samples, released exponentially and smoothed by two box filters, so it
// reaches each peak's gain before the peak: no sample leaves above the
// threshold. Delays the audio by the attack time.
class Limiter
{
  public:
    // -0.5 dBFS: margin for peaks between samples
    static constexpr float kThreshold      = 0.944f;
    static constexpr float kAttackSeconds  = 0.005f;
    static constexpr float kHoldSeconds    = 0.015f;
    static constexpr float kReleaseSeconds = 0.04f;
    static constexpr float kMaxSampleRate  = 96000.0f;

    void Init(float sample_rate);
    void Process(float& left, float& right);

  private:
    static constexpr size_t kMaxBox
        = size_t(kMaxSampleRate * kAttackSeconds / 2.0f);
    static constexpr size_t kMaxWindow
        = size_t(kMaxSampleRate * (kAttackSeconds + kHoldSeconds));

    // Minimum of the last length values: monotonic queue in a ring
    class MovingMin
    {
      public:
        void  Init(size_t length);
        float Process(float x);

      private:
        std::array<float, kMaxWindow>    value_;
        std::array<uint32_t, kMaxWindow> time_;
        size_t                           length_ = 1;
        size_t                           front_  = 0;
        size_t                           count_  = 0;
        uint32_t                         now_    = 0;
    };

    // Mean of the last length values
    class Box
    {
      public:
        void  Init(size_t length);
        float Process(float x);

      private:
        std::array<float, kMaxBox> ring_;
        size_t                     length_     = 1;
        double                     inv_length_ = 1.0;
        size_t                     pos_        = 0;
        double sum_ = 0.0; // double: float sums drift over hours
    };

    MovingMin min_;
    Box       box1_;
    Box       box2_;
    float     release_       = 1.0f;
    float     release_coeff_ = 0.0f;

    // DelayLine::SetDelay(n) after Write() returns the input n - 1 samples late
    daisysp::DelayLine<float, 2 * kMaxBox> delay_l_;
    daisysp::DelayLine<float, 2 * kMaxBox> delay_r_;
};

} // namespace clessidra
