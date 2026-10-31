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
    latch_     = latch;
    offset_    = offset;
    phase_     = 0.0;
    read_      = 0.0;
    active_    = true;
    fade_      = 1.0f;
    fade_step_ = 0.0f;
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

void Grain::FadeOut(size_t samples)
{
    fade_step_ = fade_ / float(samples);
}

void Grain::Render(double inc,
                   float* left,
                   float* right,
                   size_t size,
                   Sums&  sums,
                   double write,
                   bool   writing)
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
    float fade         = fade_;
    // Far from the write point for the whole block: no fade to work out
    const double length = double(buffer.Length());
    if(writing)
    {
        double from = latch_.position + read - write;
        from        = std::fabs(from - length * std::round(from / length));
        writing     = from < double(kHeadSamples)
                             + double(size) * (std::fabs(latch_.rate) + 1.0);
    }
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
            window           = Lookup(rise_, 1.0f - fall);
            if(percussive_ > 0.0f)
                window += percussive_ * (Lookup(decay_, fall) - window);
        }
        window *= fade;
        fade -= fade_step_;

        const double position = latch_.position + read;
        float        s        = buffer.Read(position) * window * gain;
        if(writing)
        {
            double from = position - (write + double(n));
            from        = std::fabs(from - length * std::round(from / length));
            if(from < kHeadSamples)
                s *= float(from / kHeadSamples);
        }
        left[n] += s * gain_l_;
        right[n] += s * gain_r_;
        power += s * s;
        window_sum += window;
        window_power += window * window;

        read += latch_.rate;
        phase += inc;
        if(phase >= 1.0 || fade <= 0.0f)
        {
            active_ = false;
            break;
        }
    }
    offset_ = 0;
    phase_  = phase;
    fade_   = fade;
    read_   = read;
    sums.power += power;
    sums.window += window_sum;
    sums.window_power += window_power;
}

} // namespace clessidra
