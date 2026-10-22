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

// Knobs with a second-layer function
static constexpr Knobs::Knob kSecondLayer[] = { Knobs::S31, Knobs::S32, Knobs::S34,
                                                Knobs::S35, Knobs::S36, Knobs::S37 };

ClessidraUI::ClessidraUI(Touch& touch, Engine& engine) : _touch{ touch }, _engine{ engine } {
    // Second layer at the engine defaults
    const Engine::Params defaults;
    _shift.values[Knobs::S31] = Takeover(defaults.window_shape);
    _shift.values[Knobs::S32] = Takeover(defaults.spread);
    _shift.values[Knobs::S34] = Takeover(defaults.reverb);
    _shift.values[Knobs::S35] = Takeover(0.5f + 0.5f * defaults.tone);
    _shift.values[Knobs::S36] = Takeover(defaults.mix);
    _shift.values[Knobs::S37] = Takeover(defaults.resample);
    for (size_t i = 0; i < Knobs::kNumKnobs; i++) _shown[i] = _shift[i];
}

void ClessidraUI::Init() {
    // TIM3 channel 2 drives PC7, the Seed's LED. 16-bit period: about 3.7 kHz.
    _pwm.Init(daisy::PWMHandle::Config(daisy::PWMHandle::Config::Peripheral::TIM_3));
    _pwm.Channel2().Init();
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

void ClessidraUI::Process() {
    const bool recording = _touch.pads().IsTouched(kRecordPad);
    if (recording && !_recording) _record_since = daisy::System::GetNow();
    _recording = recording;
    _engine.SetRecording(recording);

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

    // While P11 is held, the LED shows the value of the second-layer knob last turned
    for (size_t i : kSecondLayer) {
        if (std::fabs(_shift[i] - _shown[i]) < kShowStep) continue;
        _shown[i] = _shift[i];
        _shown_knob = i;
    }

    Engine::Params p;

    // S30 and S33 have no second layer: always live
    const float a = k.Value(Knobs::S30);
    const float b = _main[Knobs::S35];
    p.start = std::min(a, b);
    p.length = std::fabs(b - a);

    p.grain_ms = GrainMs(_main[Knobs::S31]);
    const bool size_centre = Detent(_main[Knobs::S31], 0.05f) == 0.5f;

    const float rand_pos = _main[Knobs::S32];
    p.rand_pos = rand_pos * rand_pos * rand_pos * rand_pos;

    const float semitones = Semitones(k.Value(Knobs::S33));
    const bool unison = semitones == 0.0f;
    p.pitch = std::exp2(semitones / 12.0f);

    // S34: random pitch, 0 .. +-24 semitones, cubic: fine near 0
    const float rand_pitch = _main[Knobs::S34];
    p.rand_pitch = 24.0f * rand_pitch * rand_pitch * rand_pitch;

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
    const bool speed_centre = !frozen && speed == 0.5f;
    p.speed = (mode == daisy::Switch3::POS_DOWN ? -4.0f : 4.0f) * speed * speed;
    p.freeze = frozen;
    p.position = _main[Knobs::S36];

    // S37: playhead flow. Off at the bottom of travel, then 0.1 .. 16 grains at once,
    // exponential.
    static constexpr float kFlowOff = 0.02f;
    const float flow_knob = _main[Knobs::S37];
    p.overlap = flow_knob < kFlowOff
                    ? 0.0f
                    : 0.1f * std::pow(Engine::kMaxOverlap / 0.1f, (flow_knob - kFlowOff) / (1.0f - kFlowOff));

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
    // Tone: centre detent off
    const float tone = Detent(_shift[Knobs::S35], 0.05f);
    const bool tone_centre = tone == 0.5f;
    p.tone = 2.0f * tone - 1.0f;
    p.mix = _shift[Knobs::S36];
    p.resample = _shift[Knobs::S37];

    _engine.SetParams(p);

    // Reaching a detent: a pulse
    const uint32_t now = daisy::System::GetNow();
    const uint8_t detents =
        (unison ? 1 : 0) | (speed_centre ? 2 : 0) | (size_centre ? 4 : 0) | (tone_centre ? 8 : 0);
    if (detents & ~_detents) _pulse_until = now + kPulseMs;
    _detents = detents;

    _led();
}

// First that applies: recording flash, detent pulse, second layer (steady, brightness =
// value last turned), grains (dim glow that follows their windows)
void ClessidraUI::_led() {
    const uint32_t now = daisy::System::GetNow();

    float brightness;
    if (_recording) {
        // From the touch: a tap shows one flash
        brightness = ((now - _record_since) / kRecordFlashMs) % 2 == 0 ? 1.0f : 0.0f;
    } else if (now < _pulse_until) {
        brightness = 1.0f;
    } else if (_shifted) {
        brightness = _shift[_shown_knob];
    } else {
        brightness = kGlow * std::min(1.0f, _engine.GrainLevel());
    }
    // Cubed: even steps to the eye
    _pwm.Channel2().Set(brightness * brightness * brightness);
}

} // namespace clessidra
