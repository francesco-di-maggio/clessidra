#include "grain.h"

#include <algorithm>
#include <cmath>
#include <Utility/dsp.h>

namespace clessidra
{
float Grain::rise_[Grain::kWindowSize + 1];
float Grain::decay_[Grain::kWindowSize + 1];

void Grain::InitWindow()
{
    for(size_t i = 0; i <= kWindowSize; i++)
    {
        const float x    = float(i) / float(kWindowSize);
        const float sine = std::sin(HALFPI_F * x);
        rise_[i]         = sine * sine;
        decay_[i]        = std::exp(-kDecay * x) * std::cos(HALFPI_F * x);
    }
}

float Grain::Lookup(const float* table, float x)
{
    const float w = x * float(kWindowSize);
    // x can round up to 1
    const size_t i = std::min(static_cast<size_t>(w), kWindowSize - 1);
    return table[i] + (w - float(i)) * (table[i + 1] - table[i]);
}

void Grain::Start(const Latch& latch, size_t offset)
{
    latch_  = latch;
    offset_ = offset;
    phase_  = 0.0;
    read_   = 0.0;
    active_ = true;
    // Equal-power pan
    const float angle = HALFPI_F * latch.pan;
    gain_l_           = std::cos(angle);
    gain_r_           = std::sin(angle);

    // Hann at 0.5. Towards 1 the peak moves to the start and the decay turns
    // exponential (percussive); towards 0 the same, reversed.
    const float skew = 2.0f * std::clamp(latch.shape, 0.0f, 1.0f) - 1.0f;
    percussive_      = std::fabs(skew);
    mirrored_        = skew < 0.0f;
    peak_            = 0.5f - (0.5f - kMinPeak) * percussive_;
    rise_scale_      = 1.0f / peak_;
    fall_scale_      = 1.0f / (1.0f - peak_);
}

void Grain::Render(double inc,
                   float* left,
                   float* right,
                   size_t size,
                   Sums&  sums)
{
    if(!active_)
        return;

    // Locals: the compiler keeps them in registers through the loop
    const Buffer& buffer = *latch_.buffer;
    double        phase  = phase_;
    double        read   = read_;
    const float   gain   = latch_.gain;
    inc /= double(latch_.size);
    float power        = 0.0f;
    float window_sum   = 0.0f;
    float window_power = 0.0f;
    for(size_t n = offset_; n < size; n++)
    {
        const float x = mirrored_ ? 1.0f - float(phase) : float(phase);
        float       window;
        if(x < peak_)
        {
            window = Lookup(rise_, x * rise_scale_);
        }
        else
        {
            const float fall = std::min(1.0f, (x - peak_) * fall_scale_);
            const float hann = Lookup(rise_, 1.0f - fall);
            window = hann + percussive_ * (Lookup(decay_, fall) - hann);
        }

        const float s = buffer.Read(latch_.position + read) * window * gain;
        left[n] += s * gain_l_;
        right[n] += s * gain_r_;
        power += s * s;
        window_sum += window;
        window_power += window * window;

        read += latch_.rate;
        phase += inc;
        if(phase >= 1.0)
        {
            active_ = false;
            break;
        }
    }
    offset_ = 0;
    phase_  = phase;
    read_   = read;
    sums.power += power;
    sums.window += window_sum;
    sums.window_power += window_power;
}

} // namespace clessidra
