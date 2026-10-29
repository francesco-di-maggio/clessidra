#include "grain.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

float Grain::_rise[Grain::kWindowSize + 1];
float Grain::_decay[Grain::kWindowSize + 1];

void Grain::init_window() {
    for (size_t i = 0; i <= kWindowSize; i++) {
        const float x = float(i) / float(kWindowSize);
        const float sine = std::sin(0.5f * float(M_PI) * x);
        _rise[i] = sine * sine;
        _decay[i] = std::exp(-kDecay * x) * std::cos(0.5f * float(M_PI) * x);
    }
}

float Grain::_lookup(const float* table, float x) {
    const float w = x * float(kWindowSize);
    // x can round up to 1
    const size_t i = std::min(static_cast<size_t>(w), kWindowSize - 1);
    return table[i] + (w - float(i)) * (table[i + 1] - table[i]);
}

void Grain::start(const Latch& latch, size_t offset) {
    _latch = latch;
    _offset = offset;
    _phase = 0.0;
    _read = 0.0;
    _active = true;
    // Equal-power pan
    const float angle = 0.5f * float(M_PI) * latch.pan;
    _gain_l = std::cos(angle);
    _gain_r = std::sin(angle);

    // Hann at 0.5. Towards 1 the peak moves to the start and the decay turns
    // exponential (percussive); towards 0 the same, reversed.
    const float skew = 2.0f * std::clamp(latch.shape, 0.0f, 1.0f) - 1.0f;
    _percussive = std::fabs(skew);
    _mirrored = skew < 0.0f;
    _peak = 0.5f - (0.5f - kMinPeak) * _percussive;
    _rise_scale = 1.0f / _peak;
    _fall_scale = 1.0f / (1.0f - _peak);
}

void Grain::render(double inc, float* left, float* right, size_t size, Sums& sums) {
    if (!_active) return;

    // Locals: the compiler keeps them in registers through the loop
    const Buffer& buffer = *_latch.buffer;
    double phase = _phase;
    double read = _read;
    const float gain = _latch.gain;
    inc /= double(_latch.size);
    float power = 0.0f;
    float window_sum = 0.0f;
    float window_power = 0.0f;
    for (size_t n = _offset; n < size; n++) {
        const float x = _mirrored ? 1.0f - float(phase) : float(phase);
        float window;
        if (x < _peak) {
            window = _lookup(_rise, x * _rise_scale);
        } else {
            const float fall = std::min(1.0f, (x - _peak) * _fall_scale);
            const float hann = _lookup(_rise, 1.0f - fall);
            window = hann + _percussive * (_lookup(_decay, fall) - hann);
        }

        const float s = buffer.read(_latch.position + read) * window * gain;
        left[n] += s * _gain_l;
        right[n] += s * _gain_r;
        power += s * s;
        window_sum += window;
        window_power += window * window;

        read += _latch.rate;
        phase += inc;
        if (phase >= 1.0) {
            _active = false;
            break;
        }
    }
    _offset = 0;
    _phase = phase;
    _read = read;
    sums.power += power;
    sums.window += window_sum;
    sums.window_power += window_power;
}

} // namespace clessidra
