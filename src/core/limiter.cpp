#include "limiter.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

void Limiter::Init(float sample_rate) {
    sample_rate = std::min(sample_rate, kMaxSampleRate);
    const size_t box = std::max<size_t>(1, size_t(sample_rate * kAttackSeconds / 2.0f));
    _box1.Init(box);
    _box2.Init(box);
    // The two boxes average over 2 * box - 1 samples: the audio is delayed to the
    // newest of them, the minimum window covers them all plus the hold.
    const size_t delay = 2 * box - 2;
    _min.Init(2 * box - 1 + size_t(sample_rate * kHoldSeconds));
    _release = 1.0f;
    _release_coeff = 1.0f - std::exp(-1.0f / (kReleaseSeconds * sample_rate));
    _delay_l.Init();
    _delay_r.Init();
    _delay_l.SetDelay(delay + 1);
    _delay_r.SetDelay(delay + 1);
}

void Limiter::Process(float& left, float& right) {
    const float peak = std::max(std::fabs(left), std::fabs(right));
    const float target = peak > kThreshold ? kThreshold / peak : 1.0f;

    const float hold = _min.Process(target);
    // Falls at once, rises exponentially
    _release = std::min(hold, _release + _release_coeff * (hold - _release));
    const float gain = _box2.Process(_box1.Process(_release));

    _delay_l.Write(left);
    _delay_r.Write(right);
    left = _delay_l.Read() * gain;
    right = _delay_r.Read() * gain;
}

void Limiter::MovingMin::Init(size_t length) {
    _length = std::clamp<size_t>(length, 1, kMaxWindow);
    _front = 0;
    _count = 0;
    _now = 0;
}

float Limiter::MovingMin::Process(float x) {
    // Expire before pushing: at most _length values in the window
    if (_count > 0 && _now - _time[_front] >= _length) {
        _front = (_front + 1) % _length;
        _count--;
    }
    // Values not below x can never be the minimum again
    while (_count > 0 && _value[(_front + _count - 1) % _length] >= x) _count--;
    const size_t back = (_front + _count) % _length;
    _value[back] = x;
    _time[back] = _now;
    _count++;
    _now++;
    return _value[_front];
}

void Limiter::Box::Init(size_t length) {
    _length = std::clamp<size_t>(length, 1, kMaxBox);
    std::fill(_ring.begin(), _ring.begin() + _length, 1.0f);
    _inv_length = 1.0 / double(_length);
    _pos = 0;
    _sum = double(_length);
}

float Limiter::Box::Process(float x) {
    _sum += double(x) - double(_ring[_pos]);
    _ring[_pos] = x;
    _pos = (_pos + 1) % _length;
    return float(_sum * _inv_length);
}

} // namespace clessidra
