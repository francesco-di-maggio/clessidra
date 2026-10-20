#include "grain.h"

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
}

void Grain::Process(const Buffer& buffer, double inc, float& left, float& right) {
    if (!_active) return;

    const float w = float(_phase) * float(kWindowSize);
    const size_t i = static_cast<size_t>(w);
    const float window = _window[i] + (w - float(i)) * (_window[i + 1] - _window[i]);

    const float s = buffer.Read(_latch.position + _read) * window;
    left += s * _gain_l;
    right += s * _gain_r;

    _read += _latch.rate;
    _phase += inc;
    if (_phase >= 1.0) _active = false;
}

} // namespace clessidra
