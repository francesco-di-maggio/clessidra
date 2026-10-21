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

    const Knobs& k = _touch.knobs();
    Engine::Params p;

    const float a = k.Value(Knobs::S30);
    const float b = k.Value(Knobs::S35);
    p.start = std::min(a, b);
    p.length = std::fabs(b - a);

    // 5 ms .. 2 s, exponential: centre = 100 ms
    p.grain_ms = 5.0f * std::pow(400.0f, Detent(k.Value(Knobs::S31), 0.05f));

    const float rand_pos = k.Value(Knobs::S32);
    p.rand_pos = rand_pos * rand_pos * rand_pos * rand_pos;

    p.pitch = std::clamp(2.0f * Detent(k.Value(Knobs::S33), 0.02f), 0.01f, 2.0f);

    const float rand_pitch = k.Value(Knobs::S34);
    p.rand_pitch = rand_pitch * rand_pitch * rand_pitch;

    // S09: up forward, centre freeze (S36 = position), down reverse
    const int mode = _touch.switches().Position(Switches::S09);
    const float speed = Detent(k.Value(Knobs::S36), 0.05f);
    p.speed = (mode == daisy::Switch3::POS_DOWN ? -4.0f : 4.0f) * speed * speed;
    p.freeze = mode == daisy::Switch3::POS_CENTER;
    p.position = k.Value(Knobs::S36);

    // 0.1 .. 8 grains at once, exponential
    p.overlap = 0.1f * std::pow(80.0f, k.Value(Knobs::S37));

    // S07: up periodic, centre half random, down random grain starts
    const int flow = _touch.switches().Position(Switches::S07);
    p.jitter = flow == daisy::Switch3::POS_UP ? 0.0f : flow == daisy::Switch3::POS_CENTER ? 0.5f : 1.0f;

    _engine.SetParams(p);
}

} // namespace clessidra
