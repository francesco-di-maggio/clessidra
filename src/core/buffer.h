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

    // After writing length samples to Data(): sets the length, fades the ends
    void         Finish(size_t length);
    float*       Data() { return mem_; }
    const float* Data() const { return mem_; }
    const float* Samples() const { return mem_; } // what plays
    size_t       Capacity() const { return capacity_; }
    size_t       Length() const { return length_; }

    // Four-point Hermite interpolation, as DaisySP's DelayLine::ReadHermite.
    // pos wraps within Length().
    float Read(double pos) const;
    // The same at sample i, below Length(), and fraction f of the next
    float Read(size_t i, float f) const;

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
    const size_t i0 = std::min(static_cast<size_t>(pos), length_ - 1);
    return Read(i0, float(pos - double(i0)));
}

inline float Buffer::Read(size_t i0, float f) const
{
    float xm1, x0, x1, x2;
    if(i0 >= 1 && i0 + 2 < length_)
    {
        xm1 = mem_[i0 - 1];
        x0  = mem_[i0];
        x1  = mem_[i0 + 1];
        x2  = mem_[i0 + 2];
    }
    else
    {
        xm1 = mem_[(i0 + length_ - 1) % length_];
        x0  = mem_[i0];
        x1  = mem_[(i0 + 1) % length_];
        x2  = mem_[(i0 + 2) % length_];
    }
    const float c     = (x1 - xm1) * 0.5f;
    const float v     = x0 - x1;
    const float w     = c + v;
    const float a     = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;
    return (((a * f) - b_neg) * f + c) * f + x0;
}

} // namespace clessidra
