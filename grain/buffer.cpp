#include "buffer.h"

#include <algorithm>

namespace clessidra {

void Buffer::Init(float* mem, size_t capacity, float sample_rate) {
    _mem = mem;
    _capacity = capacity;
    _length = 0;
    _fade_samples = std::max<size_t>(1, static_cast<size_t>(kFadeSeconds * sample_rate));
    std::fill(_mem, _mem + _capacity, 0.0f);
}

void Buffer::Finish(size_t length) {
    _length = std::min(length, _capacity);
    const size_t fade = std::min(_fade_samples, _length / 2);
    for (size_t i = 0; i < fade; i++) {
        const float gain = float(i) / float(fade);
        _mem[i] *= gain;
        _mem[_length - 1 - i] *= gain;
    }
}

void Ring::Init(float* mem, size_t capacity, float sample_rate) {
    _mem = mem;
    _capacity = capacity;
    _margin = std::max<size_t>(1, static_cast<size_t>(Buffer::kFadeSeconds * sample_rate));
    std::fill(_mem, _mem + _capacity, 0.0f);
}

void Ring::Write(float x) {
    _mem[_write_pos] = x;
    if (++_write_pos == _capacity) _write_pos = 0;
    _written = std::min(_written + 1, _capacity);
}

void Ring::Select(size_t end_back, size_t length) {
    end_back = std::min(end_back, _written);
    _length = std::min(length, _written - end_back);
    _start = (_write_pos + 2 * _capacity - end_back - _length) % _capacity;
    _done = 0;
    _copied = 0;
    _last = 0;
}

Ring::Step Ring::Copy(Buffer& dst, size_t max_samples) {
    // At most the region: _copied <= _done <= _length <= dst capacity
    const size_t end = std::min(_done + max_samples, _length);
    float* data = dst.Data();
    size_t i = _index(_done);
    // Leading silence, then from the margin before the first sound
    for (; _copied == 0 && _done < end; _done++) {
        if (std::fabs(_mem[i]) > kSilence) {
            for (size_t k = _done > _margin ? _done - _margin : 0; k < _done; k++) data[_copied++] = _mem[_index(k)];
            break;
        }
        if (++i == _capacity) i = 0;
    }
    for (; _done < end; _done++) {
        const float x = _mem[i];
        if (++i == _capacity) i = 0;
        data[_copied++] = x;
        if (std::fabs(x) > kSilence) _last = _copied;
    }
    if (_done < _length) return Step::Pending;
    if (_copied == 0) return Step::Silent;
    dst.Finish(std::min(_copied, _last + _margin));
    return Step::Done;
}

} // namespace clessidra
