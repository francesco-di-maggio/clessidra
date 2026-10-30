#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra
{
// One grain: a window over a buffer read. Buffer, position, read rate, pan,
// window shape and size are latched at Start(). The window advances at the rate
// passed to Render(), divided by the size.
class Grain
{
  public:
    struct Latch
    {
        const Buffer* buffer   = nullptr;
        double        position = 0.0;  // samples
        float         rate     = 1.0f; // samples read per output sample
        float         pan      = 0.5f; // 0 left .. 1 right
        // window: 0 reverse percussive, 0.5 Hann, 1 percussive
        float shape = 0.5f;
        float gain  = 1.0f; // the source's level
        float size  = 1.0f; // of the grain size passed to Render()
    };

    // Once, before any Grain is used.
    static void InitWindow();

    bool          IsActive() const { return active_; }
    const Buffer* Source() const { return latch_.buffer; }
    // offset: first sample of the grain in the next Render()
    void Start(const Latch& latch, size_t offset);

    // Sums over all grains and samples of a block
    struct Sums
    {
        float power        = 0.0f; // squared windowed samples, before panning
        float window       = 0.0f; // windows
        float window_power = 0.0f; // squared windows
    };

    // Adds the grain to left and right while active. inc: window phase per
    // sample.
    void Render(double inc, float* left, float* right, size_t size, Sums& sums);

  private:
    static constexpr size_t kWindowSize = 512;
    // percussive decay: e^-5 at the end, -43 dB
    static constexpr float kDecay   = 5.0f;
    static constexpr float kMinPeak = 0.01f; // percussive peak, of the grain
    // Over 0..1: Hann's rise sin^2(pi/2 x), the percussive decay e^-kx cos(pi/2
    // x)
    static float rise_[kWindowSize + 1];
    static float decay_[kWindowSize + 1];
    static float Lookup(const float* table, float x);

    Latch  latch_;
    double phase_  = 0.0;
    double read_   = 0.0; // samples read since Start()
    size_t offset_ = 0;
    float  gain_l_ = 0.0f;
    float  gain_r_ = 0.0f;
    // Window: rises to its peak, then decays as Hann blended to the percussive
    // decay
    float peak_       = 0.5f; // of the grain
    float rise_scale_ = 2.0f; // 1 / peak_
    float fall_scale_ = 2.0f; // 1 / (1 - peak_)
    float percussive_ = 0.0f;
    bool  mirrored_   = false; // percussive window reversed
    bool  active_     = false;
};

} // namespace clessidra
