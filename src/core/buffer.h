#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace clessidra {

// Mono recording in external memory, read by the grains. Filled by Ring::copy(), with
// short fades at both ends so it has no clicks at its edges.
class Buffer {
public:
    static constexpr float kFadeSeconds = 0.005f;

    // mem: capacity floats, zeroed here (SDRAM content is undefined at boot)
    void init(float* mem, size_t capacity, float sample_rate);

    // After writing length samples to data(): sets the length and fades the ends
    void finish(size_t length);
    float* data() { return _mem; }
    const float* data() const { return _mem; }
    size_t capacity() const { return _capacity; }

    // Linear interpolation. pos wraps within length().
    float read(double pos) const;

    size_t length() const { return _length; }

private:
    float* _mem = nullptr;
    size_t _capacity = 0;
    size_t _length = 0;
    size_t _fade_samples = 1;
};

// Inline: called per grain per sample
inline float Buffer::read(double pos) const {
    if (_length == 0) return 0.0f;
    const double len = double(_length);
    if (pos < 0.0 || pos >= len) pos -= len * std::floor(pos / len);

    // pos can round up to len
    const size_t i0 = std::min(static_cast<size_t>(pos), _length - 1);
    const size_t i1 = (i0 + 1 < _length) ? i0 + 1 : 0;
    const float frac = float(pos - double(i0));
    return _mem[i0] + frac * (_mem[i1] - _mem[i0]);
}

} // namespace clessidra
