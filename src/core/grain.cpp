#include "grain.h"

#include <algorithm>
#include <cmath>
#include <cstdlib> // RAND_MAX, for DaisySP dsp.h
#include <Utility/dsp.h>

namespace clessidra
{
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

    // Beads' grain envelope (beads/dsp/grain_envelope.h:57): square, decay,
    // bell, reverse along the shape. Here the grain keeps its length, so the
    // shortest rise and fall are taken from it, not added to it.
    const float shape   = std::clamp(latch.shape, 0.0f, 1.0f);
    const float sustain = std::max(1.0f - 3.0f * shape, 0.0f);
    float       attack  = std::max((shape - 1.0f / 3.0f) * 1.5f, 0.0f);
    if(attack < 0.5f)
        attack = attack * attack * attack * 4.0f;
    smoothness_     = attack < 0.5f ? 2.0f * attack : 2.0f - 2.0f * attack;
    const float inc = float(latch.inc);
    float rise = std::max(attack, kMinAttack * (1.0f + 3.0f * sustain) * inc);
    float fall
        = std::max((1.0f - attack) * (1.0f - sustain), kMinRelease * inc);
    const float both = rise + fall;
    if(both > 1.0f)
    {
        rise /= both;
        fall /= both;
    }
    attack_        = rise;
    release_from_  = 1.0f - fall;
    attack_scale_  = 1.0f / rise;
    release_scale_ = 1.0f / fall;
}

void Grain::Render(double speed,
                   float* left,
                   float* right,
                   size_t size,
                   Sums&  sums)
{
    if(!active_)
        return;

    // Locals: the compiler keeps them in registers through the loop
    const Buffer& buffer       = *latch_.buffer;
    double        phase        = phase_;
    double        read         = read_;
    const double  inc          = latch_.inc * speed;
    const double  step         = double(latch_.rate) * speed;
    float         power        = 0.0f;
    float         window_sum   = 0.0f;
    float         window_power = 0.0f;
    for(size_t n = offset_; n < size; n++)
    {
        const float x      = float(phase);
        float       window = 1.0f;
        if(x < attack_ || x >= release_from_)
        {
            // Up: straight. Down: its fourth power. Both towards a smooth
            // step.
            const bool  rising = x < attack_;
            const float t
                = rising ? x * attack_scale_
                         : std::max(1.0f - (x - release_from_) * release_scale_,
                                    0.0f);
            const float base = rising ? t : (t * t) * (t * t);
            window = base + (t * t * (3.0f - 2.0f * t) - base) * smoothness_;
        }
        const double position = latch_.position + read;
        const float  s        = buffer.Read(position) * window;
        left[n] += s * gain_l_;
        right[n] += s * gain_r_;
        power += s * s;
        window_sum += window;
        window_power += window * window;

        read += step;
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
