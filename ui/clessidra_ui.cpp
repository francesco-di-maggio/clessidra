#include "clessidra_ui.h"
#include "../config.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

// Values within width of 0.5 read as 0.5
static float Detent(float x, float width) {
    return std::fabs(x - 0.5f) <= width ? 0.5f : x;
}

void ClessidraUI::Process(daisy::DaisySeed& hw) {
    _engine.SetRecording(_touch.pads().IsTouched(kRecordPad));
    hw.SetLed(_engine.IsRecording());

    // Curves and detents from p mappings in max/patchers/granulator/granulator.maxpat.
    // S09, S07 and pad pressure are unmapped.
    const Knobs& k = _touch.knobs();
    Engine::Params p;

    const float a = k.Value(Knobs::S30);
    const float b = k.Value(Knobs::S35);
    p.start = std::min(a, b);
    p.length = std::fabs(b - a);

    const float size = Detent(k.Value(Knobs::S31), 0.05f);
    p.grain_ms = std::max(0.5f * size * size * size * _engine.RecordedSeconds() * 1000.0f, 10.0f);

    const float rand_pos = k.Value(Knobs::S32);
    p.rand_pos = rand_pos * rand_pos * rand_pos * rand_pos;

    p.pitch = std::clamp(2.0f * Detent(k.Value(Knobs::S33), 0.02f), 0.01f, 2.0f);

    const float rand_pitch = k.Value(Knobs::S34);
    p.rand_pitch = rand_pitch * rand_pitch * rand_pitch;

    const float speed = Detent(k.Value(Knobs::S36), 0.05f);
    p.speed = 4.0f * speed * speed;

    const float gain = k.Value(Knobs::S37);
    p.gain = gain * gain;

    _engine.SetParams(p);
}

} // namespace clessidra
