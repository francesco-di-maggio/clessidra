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

bool Grain::Advance(float inc) {
    _phase += inc;
    if (_phase < 1.0f) return false;
    _phase -= 1.0f;
    return true;
}

void Grain::Start(const Latch& latch) {
    _latch = latch;
    // Equal-power pan
    const float angle = 0.5f * float(M_PI) * latch.pan;
    _gain_l = std::cos(angle);
    _gain_r = std::sin(angle);
}

void Grain::Process(const Buffer& buffer, float gate_inc, float& left, float& right) {
    // Ramped play gate: a skipped grain fades instead of cutting
    if (_latch.play) {
        _gate = std::fmin(_gate + gate_inc, 1.0f);
    } else {
        _gate = std::fmax(_gate - gate_inc, 0.0f);
    }
    if (_gate == 0.0f) return;

    const float w = _phase * float(kWindowSize);
    const size_t i = static_cast<size_t>(w);
    const float window = _window[i] + (w - float(i)) * (_window[i + 1] - _window[i]);

    const float s = buffer.Read(_latch.position + _phase * _latch.span) * window * _gate;
    left += s * _gain_l;
    right += s * _gain_r;
}

} // namespace clessidra
