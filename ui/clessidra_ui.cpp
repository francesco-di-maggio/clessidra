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

// Reverb damping, 2 - 18 kHz, exponential
static constexpr float kLowpassMinHz = 2000.0f;
static constexpr float kLowpassMaxHz = 18000.0f;

static float LowpassHz(float x) {
    return kLowpassMinHz * std::pow(kLowpassMaxHz / kLowpassMinHz, x);
}

static float LowpassKnob(float hz) {
    return std::log(hz / kLowpassMinHz) / std::log(kLowpassMaxHz / kLowpassMinHz);
}

ClessidraUI::ClessidraUI(Touch& touch, Engine& engine) : _touch{ touch }, _engine{ engine } {
    // Second layer at the engine defaults. S30 and S33 have no second layer.
    const Engine::Params defaults;
    _shift.values[Knobs::S31] = Takeover(defaults.window_shape);
    _shift.values[Knobs::S32] = Takeover(defaults.spread);
    _shift.values[Knobs::S34] = Takeover(defaults.reverb);
    _shift.values[Knobs::S35] = Takeover(LowpassKnob(defaults.reverb_lowpass_hz));
    _shift.values[Knobs::S36] = Takeover(defaults.mix);
    _shift.values[Knobs::S37] = Takeover(std::sqrt(defaults.gain));
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
    for (size_t i = 0; i < Knobs::kNumKnobs; i++) values[i].Engage(k.Value(i));
}

void ClessidraUI::Layer::Process(const Knobs& k) {
    for (size_t i = 0; i < Knobs::kNumKnobs; i++) values[i].Process(k.Value(i));
}

void ClessidraUI::Process(daisy::DaisySeed& hw) {
    _engine.SetRecording(_touch.pads().IsTouched(kRecordPad));
    _led(hw);

    const Knobs& k = _touch.knobs();
    if (!_started) {
        // The main layer starts where its knobs are, and follows them exactly
        for (size_t i = 0; i < Knobs::kNumKnobs; i++) _main.values[i] = Takeover(k.Value(i));
        _started = true;
    }
    const bool shifted = _touch.pads().IsTouched(kShiftPad);
    if (shifted != _shifted) (shifted ? _shift : _main).Engage(k);
    _shifted = shifted;
    (shifted ? _shift : _main).Process(k);

    Engine::Params p;

    // S30 and S33 have no second layer: always live
    const float a = k.Value(Knobs::S30);
    const float b = _main[Knobs::S35];
    p.start = std::min(a, b);
    p.length = std::fabs(b - a);

    p.grain_ms = GrainMs(_main[Knobs::S31]);

    const float rand_pos = _main[Knobs::S32];
    p.rand_pos = rand_pos * rand_pos * rand_pos * rand_pos;

    p.pitch = std::exp2(Semitones(k.Value(Knobs::S33)) / 12.0f);

    const float rand_pitch = _main[Knobs::S34];
    p.rand_pitch = rand_pitch * rand_pitch * rand_pitch;

    // S09: up forward, centre freeze, down reverse. S36 sets the speed; while frozen it
    // scrubs, and the speed resumes after the freeze with a takeover.
    const int mode = _touch.switches().Position(Switches::S09);
    const bool frozen = mode == daisy::Switch3::POS_CENTER;
    if (frozen && !_frozen) _speed = _main[Knobs::S36];
    if (!frozen && _frozen) {
        _main.values[Knobs::S36] = Takeover(_speed);
        _main.values[Knobs::S36].Engage(k.Value(Knobs::S36));
    }
    _frozen = frozen;
    const float speed = Detent(frozen ? _speed : _main[Knobs::S36], 0.05f);
    p.speed = (mode == daisy::Switch3::POS_DOWN ? -4.0f : 4.0f) * speed * speed;
    p.freeze = frozen;
    p.position = _main[Knobs::S36];

    // S37: playhead flow, 0.1 .. 16 grains at once, exponential
    p.overlap = 0.1f * std::pow(Engine::kMaxOverlap / 0.1f, _main[Knobs::S37]);

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
    p.window_shape = _shift[Knobs::S31];
    p.spread = _shift[Knobs::S32];
    p.reverb = _shift[Knobs::S34];
    p.reverb_lowpass_hz = LowpassHz(_shift[Knobs::S35]);
    p.mix = _shift[Knobs::S36];
    const float gain = _shift[Knobs::S37];
    p.gain = gain * gain;

    _engine.SetParams(p);
}

// Recording: 4 Hz flash. Otherwise a blink per grain start.
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
