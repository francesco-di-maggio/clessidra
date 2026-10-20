#include "buffer.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

void Buffer::Init(float* mem, size_t capacity, float sample_rate) {
    _mem = mem;
    _capacity = capacity;
    _length = 0;
    _fade_samples = std::max<size_t>(1, static_cast<size_t>(kFadeSeconds * sample_rate));
    std::fill(_mem, _mem + _capacity, 0.0f);
}

void Buffer::StartRecording() {
    if (_state != State::Idle) return;
    _write_pos = 0;
    _fade_count = 0;
    _state = State::Recording;
}

void Buffer::StopRecording() {
    if (_state != State::Recording) return;
    _fade_count = std::min(_fade_count, _fade_samples);
    _state = State::FadingOut;
}

void Buffer::Write(float x) {
    if (_state == State::Idle) return;

    // Full: fade out so the recording ends at capacity.
    if (_state == State::Recording && _write_pos + _fade_samples >= _capacity) {
        StopRecording();
    }

    float gain = 1.0f;
    if (_state == State::Recording) {
        if (_fade_count < _fade_samples) {
            gain = float(_fade_count) / float(_fade_samples);
            _fade_count++;
        }
    } else {
        if (_fade_count == 0 || _write_pos >= _capacity) {
            _finish();
            return;
        }
        gain = float(_fade_count) / float(_fade_samples);
        _fade_count--;
    }

    _mem[_write_pos++] = x * gain;
}

float Buffer::Read(float pos) const {
    if (_length == 0) return 0.0f;
    const float len = float(_length);
    pos = std::fmod(pos, len);
    if (pos < 0.0f) pos += len;

    const size_t i0 = static_cast<size_t>(pos);
    const size_t i1 = (i0 + 1 < _length) ? i0 + 1 : 0;
    const float frac = pos - float(i0);
    return _mem[i0] + frac * (_mem[i1] - _mem[i0]);
}

void Buffer::_finish() {
    _length = _write_pos;
    _state = State::Idle;
}

} // namespace clessidra
