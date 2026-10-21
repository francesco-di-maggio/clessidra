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

void Grain::Start(const Latch& latch) {
    _latch = latch;
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

void Grain::Process(const Buffer& buffer, double inc, Mix& mix) {
    if (!_active) return;

    const float w = float(_phase) * float(kWindowSize);
    const size_t i = static_cast<size_t>(w);
    float window = _window[i] + (w - float(i)) * (_window[i + 1] - _window[i]);
    window = std::min(1.0f, window * _clip);
    window += _hann * (window * window - window);

    const float s = buffer.Read(_latch.position + _read) * window;
    mix.left += s * _gain_l;
    mix.right += s * _gain_r;
    mix.power += s * s;
    mix.window_power += window * window;

    _read += _latch.rate;
    _phase += inc;
    if (_phase >= 1.0) _active = false;
}

} // namespace clessidra
