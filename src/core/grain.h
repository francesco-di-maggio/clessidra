#pragma once

#include <cstddef>
#include "buffer.h"
#include "config.h"

namespace clessidra
{
// One grain: a window over a buffer read. Buffer, position, read rate, pan,
// window shape and length are latched at Start().
class Grain
{
  public:
    struct Latch
    {
        const Buffer* buffer   = nullptr;
        double        position = 0.0;  // samples
        float         rate     = 1.0f; // samples read per output sample
        float         pan      = 0.5f; // 0 left .. 1 right
        // window: 0 square, 1/3 decay, 2/3 bell, 1 reverse
        float  shape = 2.0f / 3.0f;
        double inc   = 0.0; // window phase per sample: 1 / length in samples
    };

    bool          IsActive() const { return active_; }
    const Buffer* Source() const { return latch_.buffer; }
    // offset: first sample of the grain in the next Render()
    void   Start(const Latch& latch, size_t offset);
    double Progress() const { return phase_; } // through the window, 0..1
    // For displays, between blocks: as started, and where it reads now
    const Latch& Settings() const { return latch_; }
    double       Position() const { return latch_.position + read_; }

    // Sums over all grains and samples of a block
    struct Sums
    {
        float power        = 0.0f; // squared windowed samples, before panning
        float window       = 0.0f; // windows
        float window_power = 0.0f; // squared windows
    };

    // Adds the grain to left and right while active. speed: of everything, 1
    // as started, slower as a tape machine stopping.
    void
    Render(double speed, float* left, float* right, size_t size, Sums& sums);

  private:
    // The shortest rise and fall, samples: no click at a grain's edges. A
    // grain shorter than both keeps their proportion.
    static constexpr float kMinAttack  = 0.0005f * float(kSampleRateHz);
    static constexpr float kMinRelease = 0.010f * float(kSampleRateHz);

    Latch  latch_;
    double phase_  = 0.0;
    double read_   = 0.0; // samples read since Start()
    size_t offset_ = 0;
    float  gain_l_ = 0.0f;
    float  gain_r_ = 0.0f;
    // Window: a rise, a hold at 1, a fall, as shares of the grain. Straight
    // up and steeply down, or both smooth.
    float attack_        = 0.5f;
    float release_from_  = 0.5f;
    float attack_scale_  = 2.0f; // 1 / attack_
    float release_scale_ = 2.0f; // 1 / (1 - release_from_)
    float smoothness_    = 1.0f;
    bool  active_        = false;
};

} // namespace clessidra
