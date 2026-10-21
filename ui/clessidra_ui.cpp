#include "clessidra_ui.h"
#include "../config.h"

#include <algorithm>
#include <cmath>
#include <iterator>

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

void ClessidraUI::Takeover::Process(float knob) {
    float delta = knob - _previous;
    _previous = knob;
    if (_origin >= 0.0f) {
        if (std::fabs(knob - _origin) < kDeadband) return;
        // The motion inside the deadband counts
        delta = knob - _origin;
        _origin = -1.0f;
    }
    // Towards an end stop, the value covers its remaining range while the knob covers its own
    const float scale = delta > 0.0f ? (1.001f - _value) / (1.001f - (knob - delta))
                                     : (0.001f + _value) / (0.001f + (knob - delta));
    _value = std::clamp(_value + std::clamp(scale, 0.1f, 10.0f) * delta, 0.0f, 1.0f);
}

void ClessidraUI::Layer::Engage(const Knobs& k) {
    s30.Engage(k.Value(Knobs::S30));
    s31.Engage(k.Value(Knobs::S31));
    s32.Engage(k.Value(Knobs::S32));
    s33.Engage(k.Value(Knobs::S33));
    s34.Engage(k.Value(Knobs::S34));
    s37.Engage(k.Value(Knobs::S37));
}

void ClessidraUI::Layer::Process(const Knobs& k) {
    s30.Process(k.Value(Knobs::S30));
    s31.Process(k.Value(Knobs::S31));
    s32.Process(k.Value(Knobs::S32));
    s33.Process(k.Value(Knobs::S33));
    s34.Process(k.Value(Knobs::S34));
    s37.Process(k.Value(Knobs::S37));
}

void ClessidraUI::Process(daisy::DaisySeed& hw) {
    _engine.SetRecording(_touch.pads().IsTouched(kRecordPad));
    _led(hw);

    const Knobs& k = _touch.knobs();
    if (!_started) {
        // The main layer starts where its knobs are, and follows them exactly
        _main = Layer{ Takeover(k.Value(Knobs::S30)), Takeover(k.Value(Knobs::S31)),
                       Takeover(k.Value(Knobs::S32)), Takeover(k.Value(Knobs::S33)),
                       Takeover(k.Value(Knobs::S34)), Takeover(k.Value(Knobs::S37)) };
        _started = true;
    }
    const bool shifted = _touch.pads().IsTouched(kShiftPad);
    if (shifted != _shifted) (shifted ? _shift : _main).Engage(k);
    _shifted = shifted;
    (shifted ? _shift : _main).Process(k);

    Engine::Params p;

    const float a = _main.s30.Value();
    const float b = k.Value(Knobs::S35);
    p.start = std::min(a, b);
    p.length = std::fabs(b - a);

    p.grain_ms = GrainMs(_main.s31.Value());

    const float rand_pos = _main.s32.Value();
    p.rand_pos = rand_pos * rand_pos * rand_pos * rand_pos;

    p.pitch = std::exp2(Semitones(_main.s33.Value()) / 12.0f);

    const float rand_pitch = _main.s34.Value();
    p.rand_pitch = rand_pitch * rand_pitch * rand_pitch;

    // S09: up forward, centre freeze (S36 = position), down reverse
    const int mode = _touch.switches().Position(Switches::S09);
    const float speed = Detent(k.Value(Knobs::S36), 0.05f);
    p.speed = (mode == daisy::Switch3::POS_DOWN ? -4.0f : 4.0f) * speed * speed;
    p.freeze = mode == daisy::Switch3::POS_CENTER;
    p.position = k.Value(Knobs::S36);

    // S37: playhead flow, 0.1 .. 16 grains at once, exponential
    p.overlap = 0.1f * std::pow(Engine::kMaxOverlap / 0.1f, _main.s37.Value());

    // A touched place pours 1 .. 16 grains at once, exponential in pressure
    static_assert(std::size(kPlacePads) == Engine::kNumPlaces);
    for (size_t i = 0; i < Engine::kNumPlaces; i++) {
        const size_t pad = kPlacePads[i];
        p.place_overlap[i] = _touch.pads().IsTouched(pad)
                                 ? std::pow(Engine::kMaxOverlap, _touch.pads().Pressure(pad))
                                 : 0.0f;
    }

    // S07: up periodic, centre half random, down random grain starts
    const int flow = _touch.switches().Position(Switches::S07);
    p.jitter = flow == daisy::Switch3::POS_UP ? 0.0f : flow == daisy::Switch3::POS_CENTER ? 0.5f : 1.0f;

    // P01 held: no new grains. Those playing and the reverb ring out; time keeps running.
    if (_touch.pads().IsTouched(kStopPad)) {
        p.overlap = 0.0f;
        p.place_overlap.fill(0.0f);
    }

    // P11 layer
    p.mix = _shift.s30.Value();
    p.window_shape = _shift.s31.Value();
    p.spread = _shift.s32.Value();
    p.reverb = _shift.s33.Value();
    p.chance = _shift.s34.Value();
    const float gain = _shift.s37.Value();
    p.gain = gain * gain;

    _engine.SetParams(p);
}

// Recording: a steady flash, which grain starts cannot make. Otherwise a blink per
// grain start.
void ClessidraUI::_led(daisy::DaisySeed& hw) {
    const uint32_t now = daisy::System::GetNow();
    const uint32_t starts = _engine.GrainStarts();
    if (starts != _grain_starts) _blink_start = now;
    _grain_starts = starts;
    if (_engine.IsRecording()) {
        hw.SetLed((now / kRecordFlashMs) % 2 == 0);
        return;
    }
    hw.SetLed(now - _blink_start < kBlinkMs);
}

} // namespace clessidra
