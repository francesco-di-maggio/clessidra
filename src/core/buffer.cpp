#include "buffer.h"

#include <algorithm>

namespace clessidra {

void Buffer::init(float* mem, size_t capacity, float sample_rate) {
    _mem = mem;
    _capacity = capacity;
    _length = 0;
    _fade_samples = std::max<size_t>(1, static_cast<size_t>(kFadeSeconds * sample_rate));
    std::fill(_mem, _mem + _capacity, 0.0f);
}

void Buffer::finish(size_t length) {
    _length = std::min(length, _capacity);
    const size_t fade = std::min(_fade_samples, _length / 2);
    for (size_t i = 0; i < fade; i++) {
        const float gain = float(i) / float(fade);
        _mem[i] *= gain;
        _mem[_length - 1 - i] *= gain;
    }
}

} // namespace clessidra
