#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace clessidra
{
// Mono recording in external memory, read by the grains. Filled by
// The sound the grains read, a circle of samples.
class Buffer
{
  public:
    // mem: length floats, zeroed here (SDRAM content is undefined at boot)
    void Init(float* mem, size_t length);

    float*       Data() { return mem_; }
    const float* Data() const { return mem_; }

    // Linear interpolation. pos wraps within Length().
    float Read(double pos) const;

    size_t Length() const { return length_; }

  private:
    float* mem_    = nullptr;
    size_t length_ = 0;
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
