#include "clessidra_ui.h"

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

// 10 ms .. 4 s, exponential in each half: centre = 100 ms
static float GrainMs(float x) {
    x = Detent(x, 0.05f);
    return x < 0.5f ? 10.0f * std::pow(10.0f, 2.0f * x) : 100.0f * std::pow(40.0f, 2.0f * x - 1.0f);
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
static constexpr Controls::Knob kSecondLayer[] = { Controls::S31, Controls::S33, Controls::S34,
                                                   Controls::S35, Controls::S36, Controls::S37 };

ClessidraUI::ClessidraUI() {
    // Second layer at the engine defaults
    const Engine::Params defaults;
    _shift.values[Controls::S31] = Takeover(defaults.window_shape);
    _shift.values[Controls::S33] = Takeover(defaults.harmonic);
    _shift.values[Controls::S34] = Takeover(defaults.reverb);
    _shift.values[Controls::S35] = Takeover(0.5f + 0.5f * defaults.tone);
    _shift.values[Controls::S36] = Takeover(defaults.mix);
    _shift.values[Controls::S37] = Takeover(defaults.resample);
    for (size_t i = 0; i < Controls::kNumKnobs; i++) _shown[i] = _shift[i];
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

void ClessidraUI::Layer::Engage(const Controls& c) {
    for (size_t i = 0; i < Controls::kNumKnobs; i++) values[i].Engage(c.knobs[i]);
}

void ClessidraUI::Layer::Process(const Controls& c) {
    for (size_t i = 0; i < Controls::kNumKnobs; i++) values[i].Process(c.knobs[i]);
}

void ClessidraUI::Process(const Controls& c, Engine& engine) {
    const bool recording = c.touched[kRecordPad];
    if (recording && !_recording) _record_since = c.now_ms;
    _recording = recording;
    engine.SetRecording(recording);

    if (!_started) {
        // The main layer starts where its knobs are, and follows them exactly
        for (size_t i = 0; i < Controls::kNumKnobs; i++) _main.values[i] = Takeover(c.knobs[i]);
        _started = true;
    }
    const bool shifted = c.touched[kShiftPad];
    if (shifted != _shifted) (shifted ? _shift : _main).Engage(c);
    _shifted = shifted;
    (shifted ? _shift : _main).Process(c);

    // While P11 is held, the LED shows the value of the second-layer knob last turned
    for (size_t i : kSecondLayer) {
        if (std::fabs(_shift[i] - _shown[i]) < kShowStep) continue;
        _shown[i] = _shift[i];
        _shown_knob = i;
    }

    Engine::Params p;

    // S30 and S32 have no second layer: always live
    const float a = c.knobs[Controls::S30];
    const float b = _main[Controls::S35];
    p.start = std::min(a, b);
    p.length = std::fabs(b - a);

    p.grain_ms = GrainMs(_main[Controls::S31]);
    const bool size_centre = Detent(_main[Controls::S31], 0.05f) == 0.5f;

    const float rand_pos = c.knobs[Controls::S32];
    p.rand_pos = rand_pos * rand_pos * rand_pos * rand_pos;

    const float semitones = Semitones(_main[Controls::S33]);
    const bool unison = semitones == 0.0f;
    p.pitch = std::exp2(semitones / 12.0f);

    // S34: random pitch, 0 .. +-24 semitones, cubic: fine near 0
    const float rand_pitch = _main[Controls::S34];
    p.rand_pitch = 24.0f * rand_pitch * rand_pitch * rand_pitch;

    // S09: up forward, centre freeze, down reverse. S36 sets the speed; while frozen it
    // scrubs, and the speed resumes after the freeze with a takeover.
    const Controls::Position mode = c.switches[Controls::S09];
    const bool frozen = mode == Controls::Centre;
    if (frozen && !_frozen) _speed = _main[Controls::S36];
    if (!frozen && _frozen) {
        _main.values[Controls::S36] = Takeover(_speed);
        _main.values[Controls::S36].Engage(c.knobs[Controls::S36]);
    }
    _frozen = frozen;
    const float speed = Detent(frozen ? _speed : _main[Controls::S36], 0.05f);
    const bool speed_centre = !frozen && speed == 0.5f;
    p.speed = (mode == Controls::Down ? -4.0f : 4.0f) * speed * speed;
    p.freeze = frozen;
    p.position = _main[Controls::S36];

    // S37: playhead flow. Off at the bottom of travel, then 0.1 .. 16 grains at once,
    // exponential.
    static constexpr float kFlowOff = 0.02f;
    const float flow_knob = _main[Controls::S37];
    p.flow = flow_knob < kFlowOff
                    ? 0.0f
                    : 0.1f * std::pow(Engine::kMaxFlow / 0.1f, (flow_knob - kFlowOff) / (1.0f - kFlowOff));

    // A touched place pours 1 .. 16 grains at once, exponential in pressure. The
    // playhead gives way to the strongest press.
    static_assert(std::size(kPlacePads) == Engine::kNumPlaces);
    float press = 0.0f;
    for (size_t i = 0; i < Engine::kNumPlaces; i++) {
        const size_t pad = kPlacePads[i];
        if (!c.touched[pad]) {
            p.place_flow[i] = 0.0f;
            continue;
        }
        p.place_flow[i] = std::pow(Engine::kMaxFlow, c.pressure[pad]);
        press = std::max(press, c.pressure[pad]);
    }
    p.flow *= 1.0f - std::clamp(press, 0.0f, 1.0f);

    // S07: up periodic, centre half random, down random grain starts
    const Controls::Position starts = c.switches[Controls::S07];
    p.jitter = starts == Controls::Up ? 0.0f : starts == Controls::Centre ? 0.5f : 1.0f;

    // P01 held: no new grains. Those playing and the reverb ring out; time keeps running.
    if (c.touched[kStopPad]) {
        p.flow = 0.0f;
        p.place_flow.fill(0.0f);
    }

    // P11 layer
    // Window shape: centre detent Hann
    const float shape = Detent(_shift[Controls::S31], 0.05f);
    const bool shape_centre = shape == 0.5f;
    p.window_shape = shape;
    p.harmonic = _shift[Controls::S33];
    p.reverb = _shift[Controls::S34];
    // Tone: centre detent off
    const float tone = Detent(_shift[Controls::S35], 0.05f);
    const bool tone_centre = tone == 0.5f;
    p.tone = 2.0f * tone - 1.0f;
    p.mix = _shift[Controls::S36];
    p.resample = _shift[Controls::S37];

    engine.SetParams(p);

    // Reaching a detent, or turning within it: a pulse
    const std::array<bool, kNumDetents> at = { unison, speed_centre, size_centre, tone_centre, shape_centre };
    const std::array<float, kNumDetents> knobs = { _main[Controls::S33], _main[Controls::S36], _main[Controls::S31],
                                                   _shift[Controls::S35], _shift[Controls::S31] };
    if (!_detents_ready) {
        // Power-up: no pulse
        _at_detent = at;
        _detent_knob = knobs;
        _detents_ready = true;
    }
    for (size_t i = 0; i < kNumDetents; i++) {
        const bool moved = std::fabs(knobs[i] - _detent_knob[i]) > kPulseStep;
        if (at[i] && (!_at_detent[i] || moved)) _pulse_until = c.now_ms + kPulseMs;
        if (!at[i] || !_at_detent[i] || moved) _detent_knob[i] = knobs[i];
        _at_detent[i] = at[i];
    }
}

// First that applies: recording flash, detent pulse, second layer (steady, brightness =
// value last turned), grains (dim glow that follows their windows)
float ClessidraUI::Led(uint32_t now, const Engine& engine) const {
    float brightness;
    if (_recording) {
        // From the touch: a tap shows one flash
        brightness = ((now - _record_since) / kRecordFlashMs) % 2 == 0 ? 1.0f : 0.0f;
    } else if (now < _pulse_until) {
        brightness = 1.0f;
    } else if (_shifted) {
        brightness = _shift[_shown_knob];
    } else {
        brightness = kGlow * std::min(1.0f, engine.GrainLevel());
    }
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
