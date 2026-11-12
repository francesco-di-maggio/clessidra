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
    const Buffer& buffer = *latch_.buffer;
    const size_t  length = buffer.Length();
    double        phase  = phase_;
    const double  inc    = latch_.inc * speed;
    const double  step   = double(latch_.rate) * speed;
    if(length == 0)
    {
        active_ = false;
        return;
    }
    // Where it reads: a sample and a fraction of the next, each sample a
    // whole step and a fraction (as Clouds' grains, in fixed point). A float
    // fraction keeps its precision however far it reads.
    const double len   = double(length);
    double       start = latch_.position + read_;
    start -= len * std::floor(start / len);
    size_t        index      = std::min(size_t(start), length - 1);
    float         fraction   = float(start - double(index));
    const double  whole      = std::floor(step);
    const float   step_part  = float(step - whole);
    const int32_t step_whole = int32_t(whole);
    const auto    wrapped    = [length](int32_t i) {
        i %= int32_t(length);
        return size_t(i < 0 ? i + int32_t(length) : i);
    };
    // Members as locals too: the stores to left and right could alias them
    const float attack        = attack_;
    const float release_from  = release_from_;
    const float attack_scale  = attack_scale_;
    const float release_scale = release_scale_;
    const float smoothness    = smoothness_;
    const float gain_l        = gain_l_;
    const float gain_r        = gain_r_;
    float       power         = 0.0f;
    float       window_sum    = 0.0f;
    float       window_power  = 0.0f;
    size_t      rendered      = 0;
    for(size_t n = offset_; n < size; n++)
    {
        const float x      = float(phase);
        float       window = 1.0f;
        if(x < attack || x >= release_from)
        {
            // Up: straight. Down: its fourth power. Both towards a smooth
            // step.
            const bool  rising = x < attack;
            const float t
                = rising ? x * attack_scale
                         : std::max(1.0f - (x - release_from) * release_scale,
                                    0.0f);
            const float base = rising ? t : (t * t) * (t * t);
            window = base + (t * t * (3.0f - 2.0f * t) - base) * smoothness;
        }
        const float s = buffer.Read(index, fraction) * window;
        left[n] += s * gain_l;
        right[n] += s * gain_r;
        power += s * s;
        window_sum += window;
        window_power += window * window;

        fraction += step_part;
        int32_t next = int32_t(index) + step_whole;
        if(fraction >= 1.0f)
        {
            fraction -= 1.0f;
            next++;
        }
        index = next >= 0 && next < int32_t(length) ? size_t(next)
                                                    : wrapped(next);
        rendered++;
        phase += inc;
        if(phase >= 1.0)
        {
            active_ = false;
            break;
        }
    }
    offset_ = 0;
    phase_  = phase;
    read_ += step * double(rendered);
    sums.power += power;
    sums.window += window_sum;
    sums.window_power += window_power;
}

} // namespace clessidra
