#include "buffer.h"

#include <algorithm>

namespace clessidra
{
void Buffer::Init(float* mem, size_t capacity, float sample_rate)
{
    mem_      = mem;
    capacity_ = capacity;
    length_   = 0;
    fade_samples_
        = std::max<size_t>(1, static_cast<size_t>(kFadeSeconds * sample_rate));
    std::fill(mem_, mem_ + capacity_, 0.0f);
}

void Buffer::Finish(size_t length)
{
    length_           = std::min(length, capacity_);
    const size_t fade = std::min(fade_samples_, length_ / 2);
    for(size_t i = 0; i < fade; i++)
    {
        const float gain = float(i) / float(fade);
        mem_[i] *= gain;
        mem_[length_ - 1 - i] *= gain;
    }
}

} // namespace clessidra
