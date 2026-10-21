#include "clessidra_ui.h"
#include "../config.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

// Values within width of 0.5 read as 0.5; the rest is stretched to 0..1 without a jump
static float Detent(float x, float width) {
    if (x < 0.5f - width) return 0.5f * x / (0.5f - width);
    if (x > 0.5f + width) return 0.5f + 0.5f * (x - 0.5f - width) / (0.5f - width);
    return 0.5f;
}

// 20 ms .. 2 s, exponential in each half: centre = 100 ms
static float GrainMs(float x) {
    x = Detent(x, 0.05f);
    return x < 0.5f ? 20.0f * std::pow(5.0f, 2.0f * x) : 100.0f * std::pow(20.0f, 2.0f * x - 1.0f);
}

// -24 .. +24 semitones over equal knob segments between musical intervals, flat
// near each interval
static float Semitones(float x) {
    static constexpr float kIntervals[] = { -24.0f, -12.0f, -7.0f, 0.0f, 7.0f, 12.0f, 24.0f };
    static constexpr size_t kSegments = 6;
    static constexpr float kFlat = 0.15f; // of a segment, on each side
    const float pos = std::clamp(x, 0.0f, 1.0f) * float(kSegments);
    const size_t i = std::min(size_t(pos), kSegments - 1);
    const float t = std::clamp((pos - float(i) - kFlat) / (1.0f - 2.0f * kFlat), 0.0f, 1.0f);
    return kIntervals[i] + t * (kIntervals[i + 1] - kIntervals[i]);
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

    p.grain_ms = GrainMs(k.Value(Knobs::S31));

    const float rand_pos = k.Value(Knobs::S32);
    p.rand_pos = rand_pos * rand_pos * rand_pos * rand_pos;

    p.pitch = std::exp2(Semitones(k.Value(Knobs::S33)) / 12.0f);

    const float rand_pitch = k.Value(Knobs::S34);
    p.rand_pitch = rand_pitch * rand_pitch * rand_pitch;

    // S09: up forward, centre freeze (S36 = position), down reverse
    const int mode = _touch.switches().Position(Switches::S09);
    const float speed = Detent(k.Value(Knobs::S36), 0.05f);
    p.speed = (mode == daisy::Switch3::POS_DOWN ? -4.0f : 4.0f) * speed * speed;
    p.freeze = mode == daisy::Switch3::POS_CENTER;
    p.position = k.Value(Knobs::S36);

    // S37 sets the flow with no pad touched: 0.1 .. 16 grains at once, exponential.
    // The hardest pressed pad raises it towards 16, exponentially.
    const float flow_floor = 0.1f * std::pow(Engine::kMaxOverlap / 0.1f, k.Value(Knobs::S37));
    float pressure = 0.0f;
    for (size_t pad : kPourPads) pressure = std::max(pressure, _touch.pads().Pressure(pad));
    p.overlap = flow_floor * std::pow(Engine::kMaxOverlap / flow_floor, pressure);

    // S07: up periodic, centre half random, down random grain starts
    const int flow = _touch.switches().Position(Switches::S07);
    p.jitter = flow == daisy::Switch3::POS_UP ? 0.0f : flow == daisy::Switch3::POS_CENTER ? 0.5f : 1.0f;

    _engine.SetParams(p);
}

} // namespace clessidra
