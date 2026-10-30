#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace clessidra
{
// Mono recording in external memory, read by the grains. Filled by
// Ring::Copy(), with short fades at both ends so it has no clicks at its edges.
class Buffer
{
  public:
    static constexpr float kFadeSeconds = 0.005f;

    // mem: capacity floats, zeroed here (SDRAM content is undefined at boot)
    void Init(float* mem, size_t capacity, float sample_rate);

    // After writing length samples to Data(): sets the length and fades the
    // ends
    void         Finish(size_t length);
    float*       Data() { return mem_; }
    const float* Data() const { return mem_; }
    size_t       Capacity() const { return capacity_; }

    // Linear interpolation. pos wraps within Length().
    float Read(double pos) const;

    size_t Length() const { return length_; }

  private:
    float* mem_          = nullptr;
    size_t capacity_     = 0;
    size_t length_       = 0;
    size_t fade_samples_ = 1;
};

// Inline: called per grain per sample
inline float Buffer::Read(double pos) const
{
    if(length_ == 0)
        return 0.0f;
    const double len = double(length_);
    if(pos < 0.0 || pos >= len)
        pos -= len * std::floor(pos / len);

    // pos can round up to len
    const size_t i0   = std::min(static_cast<size_t>(pos), length_ - 1);
    const size_t i1   = (i0 + 1 < length_) ? i0 + 1 : 0;
    const float  frac = float(pos - double(i0));
    return mem_[i0] + frac * (mem_[i1] - mem_[i0]);
}

} // namespace clessidra
