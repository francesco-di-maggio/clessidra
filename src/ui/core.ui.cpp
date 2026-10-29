#include "core.ui.h"

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

// S37: off at the bottom of travel, then 0.1 .. 16 grains at once, exponential
static constexpr float kDensityOff = 0.02f;
static constexpr float kDensityMin = 0.1f;

static float Density(float x) {
    if (x < kDensityOff) return 0.0f;
    return kDensityMin * std::pow(Core::kMaxDensity / kDensityMin, (x - kDensityOff) / (1.0f - kDensityOff));
}

static float DensityKnob(float density) {
    return kDensityOff + (1.0f - kDensityOff) * std::log(density / kDensityMin) / std::log(Core::kMaxDensity / kDensityMin);
}

// 10 ms .. top_ms, exponential in each half: centre = 100 ms
static float GrainMs(float x, float top_ms) {
    x = Detent(x, 0.05f);
    return x < 0.5f ? 10.0f * std::pow(10.0f, 2.0f * x) : 100.0f * std::pow(top_ms / 100.0f, 2.0f * x - 1.0f);
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

CoreUI::CoreUI() {
    Controls c;
    _reset(c);
    // Levels at unity
    _levels.values[Controls::S36] = Takeover(0.5f);
    _levels.values[Controls::S37] = Takeover(1.0f);
    _shift_shown.update(_shift);
    _shift_shown.knob = Controls::S36;
    _levels_shown.update(_levels);
    _levels_shown.knob = Controls::S37;
}

void CoreUI::_reset(const Controls& c) {
    // At the core defaults: the recording played as it is
    const Core::Params d;
    const std::array<float, Controls::kNumKnobs> main = {
        d.start, 0.5f /* 100 ms */, 0.0f /* rand_pos */, 0.5f /* unison */, 0.0f /* rand_pitch */,
        d.start + d.length, 0.5f /* speed 1 */, DensityKnob(d.density)
    };
    const std::array<float, Controls::kNumKnobs> shift = {
        d.reverse, d.window_shape, d.size_variation, d.reverb, d.harmonic, 0.5f + 0.5f * d.tone, d.mix, d.resample
    };
    for (size_t i = 0; i < Controls::kNumKnobs; i++) {
        // While frozen S36 scrubs: the speed resumes at 1 instead
        if (!(_frozen && i == Controls::S36)) _main.values[i] = Takeover(main[i]);
        _shift.values[i] = Takeover(shift[i]);
    }
    _speed = 0.5f;
    _layer->engage(c);
}

void CoreUI::Shown::update(const Layer& layer) {
    for (size_t i = 0; i < Controls::kNumKnobs; i++) {
        if (std::fabs(layer[i] - values[i]) < kShowStep) continue;
        values[i] = layer[i];
        knob = i;
    }
}

void CoreUI::Layer::engage(const Controls& c) {
    for (size_t i = 0; i < Controls::kNumKnobs; i++) values[i].engage(c.knobs[i]);
}

void CoreUI::Layer::process(const Controls& c) {
    for (size_t i = 0; i < Controls::kNumKnobs; i++) values[i].process(c.knobs[i]);
}

void CoreUI::process(const Controls& c, Core& core) {
    const bool recording = c.touched[kRecordPad];
    if (recording && !_recording) _record_since = c.now_ms;
    _recording = recording;
    core.set_recording(recording);

    if (!_started) {
        // The main layer starts where its knobs are, and follows them exactly
        for (size_t i = 0; i < Controls::kNumKnobs; i++) _main.values[i] = Takeover(c.knobs[i]);
        _started = true;
    }
    // P11 held: second layer. P00 held: levels.
    const bool shifted = c.touched[kShiftPad];
    const bool levels = !shifted && c.touched[kLevelsPad];
    Layer* layer = shifted ? &_shift : levels ? &_levels : &_main;
    if (layer != _layer) layer->engage(c);
    _layer = layer;
    _shifted = shifted;
    _levels_on = levels;
    layer->process(c);

    // P01 tapped: defaults, the knobs taking over from there
    const bool reset = c.touched[kResetPad];
    if (reset && !_reset_touched) {
        _reset(c);
        _pulse_until = c.now_ms + kPulseMs;
    }
    _reset_touched = reset;

    _shift_shown.update(_shift);
    _levels_shown.update(_levels);

    Core::Params p;

    const float a = _main[Controls::S30];
    const float b = _main[Controls::S35];
    p.start = std::min(a, b);
    p.length = std::fabs(b - a);

    // Grain size: up to half the recording
    const float top_ms = std::max(100.0f, 500.0f * core.recorded_seconds());
    p.grain_ms = GrainMs(_main[Controls::S31], top_ms);
    const bool size_centre = Detent(_main[Controls::S31], 0.05f) == 0.5f;

    const float rand_pos = _main[Controls::S32];
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
        _main.values[Controls::S36].engage(c.knobs[Controls::S36]);
    }
    _frozen = frozen;
    const float speed = Detent(frozen ? _speed : _main[Controls::S36], 0.05f);
    const bool speed_centre = !frozen && speed == 0.5f;
    p.speed = (mode == Controls::Down ? -4.0f : 4.0f) * speed * speed;
    p.freeze = frozen;
    p.position = _main[Controls::S36];

    // S37: playhead density
    p.density = Density(_main[Controls::S37]);

    // A touched place pours at the playhead's density, or the default with the playhead
    // off. Pressure crossfades, equal power: the place up, the playhead down under the
    // strongest press.
    static_assert(std::size(kPlacePads) == Core::kNumPlaces);
    const float place_density = p.density > 0.0f ? p.density : Core::Params{}.density;
    float press = 0.0f;
    for (size_t i = 0; i < Core::kNumPlaces; i++) {
        const size_t pad = kPlacePads[i];
        if (!c.touched[pad]) {
            p.place_density[i] = 0.0f;
            continue;
        }
        const float pressure = std::clamp(c.pressure[pad], 0.0f, 1.0f);
        p.place_density[i] = place_density;
        p.place_level[i] = std::sin(0.5f * float(M_PI) * pressure);
        press = std::max(press, pressure);
    }
    p.level = std::cos(0.5f * float(M_PI) * press);

    // S07: up periodic, centre half random, down random grain starts
    const Controls::Position starts = c.switches[Controls::S07];
    p.jitter = starts == Controls::Up ? 0.0f : starts == Controls::Centre ? 0.5f : 1.0f;

    // P02 held: no new grains. Those playing and the reverb ring out; time keeps running.
    if (c.touched[kStopPad]) {
        p.density = 0.0f;
        p.place_density.fill(0.0f);
    }

    // P11 layer
    p.reverse = _shift[Controls::S30];
    // Shape: centre detent Hann
    const float shape = Detent(_shift[Controls::S31], 0.05f);
    const bool shape_centre = shape == 0.5f;
    p.window_shape = shape;
    p.size_variation = _shift[Controls::S32];
    p.reverb = _shift[Controls::S33];
    p.harmonic = _shift[Controls::S34];
    // Tone: centre detent off
    const float tone = Detent(_shift[Controls::S35], 0.05f);
    const bool tone_centre = tone == 0.5f;
    p.tone = 2.0f * tone - 1.0f;
    p.mix = _shift[Controls::S36];
    p.resample = _shift[Controls::S37];

    // Levels: input, centre detent unity. Output, fader law.
    const float input = Detent(_levels[Controls::S36], 0.05f);
    const bool input_centre = input == 0.5f;
    p.input = kMaxInput * input * input;
    const float output = _levels[Controls::S37];
    p.output = output * output;

    core.set_params(p);

    // Reaching a detent, or turning within it: a pulse
    const std::array<bool, kNumDetents> at = { unison, speed_centre, size_centre, tone_centre, shape_centre, input_centre };
    const std::array<float, kNumDetents> knobs = { _main[Controls::S33], _main[Controls::S36], _main[Controls::S31],
                                                   _shift[Controls::S35], _shift[Controls::S31], _levels[Controls::S36] };
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
float CoreUI::led(uint32_t now, const Core& core) const {
    float brightness;
    if (_recording) {
        // From the touch: a tap shows one flash
        brightness = ((now - _record_since) / kRecordFlashMs) % 2 == 0 ? 1.0f : 0.0f;
    } else if (now < _pulse_until) {
        brightness = 1.0f;
    } else if (_shifted) {
        brightness = _shift[_shift_shown.knob];
    } else if (_levels_on) {
        brightness = _levels[_levels_shown.knob];
    } else {
        brightness = kGlow * std::min(1.0f, core.grain_level());
    }
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
