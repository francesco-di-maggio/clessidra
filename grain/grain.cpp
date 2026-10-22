#include "grain.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

float Grain::_window[Grain::kWindowSize + 1];

void Grain::InitWindow() {
    // sin(pi * phase), 0 at both ends
    for (size_t i = 0; i <= kWindowSize; i++) {
        _window[i] = std::sin(float(M_PI) * float(i) / float(kWindowSize));
    }
}

void Grain::Start(const Latch& latch, size_t offset) {
    _latch = latch;
    _offset = offset;
    _phase = 0.0;
    _read = 0.0;
    _active = true;
    // Equal-power pan
    const float angle = 0.5f * float(M_PI) * latch.pan;
    _gain_l = std::cos(angle);
    _gain_r = std::sin(angle);

    // Below 0.5 the sine is scaled up to 16 and clipped: flat top, edges down to 2 % of
    // the grain each. Above 0.5 it blends to Hann.
    const float shape = std::clamp(latch.shape, 0.0f, 1.0f);
    _clip = shape < 0.5f ? std::exp2(4.0f * (1.0f - 2.0f * shape)) : 1.0f;
    _hann = shape > 0.5f ? 2.0f * shape - 1.0f : 0.0f;
}

void Grain::Render(const Buffer& buffer, double inc, float* left, float* right, size_t size, Sums& sums) {
    if (!_active) return;

    // Locals: the compiler keeps them in registers through the loop
    double phase = _phase;
    double read = _read;
    float power = 0.0f;
    float window_sum = 0.0f;
    float window_power = 0.0f;
    for (size_t n = _offset; n < size; n++) {
        const float w = float(phase) * float(kWindowSize);
        // float(phase) can round up to 1
        const size_t i = std::min(static_cast<size_t>(w), kWindowSize - 1);
        float window = _window[i] + (w - float(i)) * (_window[i + 1] - _window[i]);
        window = std::min(1.0f, window * _clip);
        window += _hann * (window * window - window);

        const float s = buffer.Read(_latch.position + read) * window;
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
