#include "core.ui.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <Utility/dsp.h>

namespace clessidra
{
namespace
{
    // Values within width of 0.5 read as 0.5; the rest is stretched to 0..1
    // without a jump
    float Detent(float x, float width)
    {
        if(x < 0.5f - width)
            return 0.5f * x / (0.5f - width);
        if(x > 0.5f + width)
            return 0.5f + 0.5f * (x - 0.5f - width) / (0.5f - width);
        return 0.5f;
    }

    // S37: off at the bottom of travel, then 0.1 .. 16 grains at once,
    // exponential
    constexpr float kDensityOff = 0.02f;
    constexpr float kDensityMin = 0.1f;

    float Density(float x)
    {
        if(x < kDensityOff)
            return 0.0f;
        return kDensityMin
               * std::pow(Core::kMaxDensity / kDensityMin,
                          (x - kDensityOff) / (1.0f - kDensityOff));
    }

    float DensityKnob(float density)
    {
        return kDensityOff
               + (1.0f - kDensityOff) * std::log(density / kDensityMin)
                     / std::log(Core::kMaxDensity / kDensityMin);
    }

    // 10 ms .. top_ms, exponential in each half: centre = 100 ms
    float GrainMs(float x, float top_ms)
    {
        x = Detent(x, 0.05f);
        return x < 0.5f ? 10.0f * std::pow(10.0f, 2.0f * x)
                        : 100.0f * std::pow(top_ms / 100.0f, 2.0f * x - 1.0f);
    }

    // -24 .. +24 semitones over equal knob segments between musical intervals,
    // flat near each interval
    float Semitones(float x)
    {
        static constexpr float kIntervals[]
            = {-24.0f, -12.0f, -7.0f, 0.0f, 7.0f, 12.0f, 24.0f};
        static constexpr size_t kSegments = 6;
        static constexpr float  kFlat     = 0.15f; // of a segment, on each side
        const float  pos = std::clamp(x, 0.0f, 1.0f) * float(kSegments);
        const size_t i   = std::min(size_t(pos), kSegments - 1);
        const float  t   = std::clamp(
            (pos - float(i) - kFlat) / (1.0f - 2.0f * kFlat), 0.0f, 1.0f);
        return kIntervals[i] + t * (kIntervals[i + 1] - kIntervals[i]);
    }

} // namespace

CoreUI::CoreUI()
{
    Controls c;
    Reset(c);
    // Levels at unity
    levels_.values[Controls::S36] = Takeover(0.5f);
    levels_.values[Controls::S37] = Takeover(1.0f);
    shift_shown_.Update(shift_);
    shift_shown_.knob = Controls::S36;
    levels_shown_.Update(levels_);
    levels_shown_.knob = Controls::S37;
}

void CoreUI::Reset(const Controls& c)
{
    // At the core defaults: the recording played as it is
    const Core::Params                           d;
    const std::array<float, Controls::kNumKnobs> main
        = {d.start,
           0.5f /* 100 ms */,
           0.0f /* rand_pos */,
           0.5f /* unison */,
           0.0f /* rand_pitch */,
           d.start + d.length,
           0.5f /* speed 1 */,
           DensityKnob(d.density)};
    const std::array<float, Controls::kNumKnobs> shift = {d.reverse,
                                                          d.window_shape,
                                                          d.size_variation,
                                                          d.reverb,
                                                          d.harmonic,
                                                          0.5f + 0.5f * d.tone,
                                                          d.mix,
                                                          d.resample};
    for(size_t i = 0; i < Controls::kNumKnobs; i++)
    {
        // While frozen S36 scrubs: the speed resumes at 1 instead
        if(!(frozen_ && i == Controls::S36))
            main_.values[i] = Takeover(main[i]);
        shift_.values[i] = Takeover(shift[i]);
    }
    speed_ = 0.5f;
    layer_->Engage(c);
}

void CoreUI::Shown::Update(const Layer& layer)
{
    for(size_t i = 0; i < Controls::kNumKnobs; i++)
    {
        if(std::fabs(layer[i] - values[i]) < kShowStep)
            continue;
        values[i] = layer[i];
        knob      = i;
    }
}

void CoreUI::Layer::Engage(const Controls& c)
{
    for(size_t i = 0; i < Controls::kNumKnobs; i++)
        values[i].Engage(c.knobs[i]);
}

void CoreUI::Layer::Process(const Controls& c)
{
    for(size_t i = 0; i < Controls::kNumKnobs; i++)
        values[i].Process(c.knobs[i]);
}

void CoreUI::Process(const Controls& c, Core& core)
{
    const bool recording = c.touched[kRecordPad];
    if(recording && !recording_)
        record_since_ = c.now_ms;
    recording_ = recording;
    core.SetRecording(recording);

    if(!started_)
    {
        // The main layer starts where its knobs are, and follows them exactly
        for(size_t i = 0; i < Controls::kNumKnobs; i++)
            main_.values[i] = Takeover(c.knobs[i]);
        started_ = true;
    }
    // P11 held: second layer. P00 held: levels.
    const bool shifted = c.touched[kShiftPad];
    const bool levels  = !shifted && c.touched[kLevelsPad];
    Layer*     layer   = shifted ? &shift_ : levels ? &levels_ : &main_;
    if(layer != layer_)
        layer->Engage(c);
    layer_     = layer;
    shifted_   = shifted;
    levels_on_ = levels;
    layer->Process(c);

    // P01 tapped: defaults, the knobs taking over from there
    const bool reset = c.touched[kResetPad];
    if(reset && !reset_touched_)
    {
        Reset(c);
        pulse_until_ = c.now_ms + kPulseMs;
    }
    reset_touched_ = reset;

    shift_shown_.Update(shift_);
    levels_shown_.Update(levels_);

    Core::Params p;

    const float a = main_[Controls::S30];
    const float b = main_[Controls::S35];
    p.start       = std::min(a, b);
    p.length      = std::fabs(b - a);

    // Grain size: up to half the recording
    const float top_ms     = std::max(100.0f, 500.0f * core.RecordedSeconds());
    p.grain_ms             = GrainMs(main_[Controls::S31], top_ms);
    const bool size_centre = Detent(main_[Controls::S31], 0.05f) == 0.5f;

    const float rand_pos = main_[Controls::S32];
    p.rand_pos           = rand_pos * rand_pos * rand_pos * rand_pos;

    const float semitones = Semitones(main_[Controls::S33]);
    const bool  unison    = semitones == 0.0f;
    p.pitch               = std::exp2(semitones / 12.0f);

    // S34: random pitch, 0 .. +-24 semitones, cubic: fine near 0
    const float rand_pitch = main_[Controls::S34];
    p.rand_pitch           = 24.0f * rand_pitch * rand_pitch * rand_pitch;

    // S09: up forward, centre freeze, down reverse. S36 sets the speed; while
    // frozen it scrubs, and the speed resumes after the freeze with a takeover.
    const Controls::Position mode   = c.switches[Controls::S09];
    const bool               frozen = mode == Controls::Centre;
    if(frozen && !frozen_)
        speed_ = main_[Controls::S36];
    if(!frozen && frozen_)
    {
        main_.values[Controls::S36] = Takeover(speed_);
        main_.values[Controls::S36].Engage(c.knobs[Controls::S36]);
    }
    frozen_           = frozen;
    const float speed = Detent(frozen ? speed_ : main_[Controls::S36], 0.05f);
    const bool  speed_centre = !frozen && speed == 0.5f;
    p.speed    = (mode == Controls::Down ? -4.0f : 4.0f) * speed * speed;
    p.freeze   = frozen;
    p.position = main_[Controls::S36];

    // S37: playhead density
    p.density = Density(main_[Controls::S37]);

    // A touched place pours at the playhead's density, or the default with the
    // playhead off. Pressure crossfades, equal power: the place up, the
    // playhead down under the strongest press.
    static_assert(std::size(kPlacePads) == Core::kNumPlaces);
    const float place_density
        = p.density > 0.0f ? p.density : Core::Params{}.density;
    float press = 0.0f;
    for(size_t i = 0; i < Core::kNumPlaces; i++)
    {
        const size_t pad = kPlacePads[i];
        if(!c.touched[pad])
        {
            p.place_density[i] = 0.0f;
            continue;
        }
        const float pressure = std::clamp(c.pressure[pad], 0.0f, 1.0f);
        p.place_density[i]   = place_density;
        p.place_level[i]     = std::sin(HALFPI_F * pressure);
        press                = std::max(press, pressure);
    }
    p.level = std::cos(HALFPI_F * press);

    // S07: up periodic, centre half random, down random grain starts
    const Controls::Position starts = c.switches[Controls::S07];
    p.jitter                        = starts == Controls::Up
                   ? 0.0f
                   : starts == Controls::Centre ? 0.5f : 1.0f;

    // P02 held: no new grains. Those playing and the reverb ring out; time
    // keeps running.
    if(c.touched[kStopPad])
    {
        p.density = 0.0f;
        p.place_density.fill(0.0f);
    }

    // P11 layer
    p.reverse = shift_[Controls::S30];
    // Shape: centre detent Hann
    const float shape        = Detent(shift_[Controls::S31], 0.05f);
    const bool  shape_centre = shape == 0.5f;
    p.window_shape           = shape;
    p.size_variation         = shift_[Controls::S32];
    p.reverb                 = shift_[Controls::S33];
    p.harmonic               = shift_[Controls::S34];
    // Tone: centre detent off
    const float tone        = Detent(shift_[Controls::S35], 0.05f);
    const bool  tone_centre = tone == 0.5f;
    p.tone                  = 2.0f * tone - 1.0f;
    p.mix                   = shift_[Controls::S36];
    p.resample              = shift_[Controls::S37];

    // Levels: input, centre detent unity. Output, fader law.
    const float input        = Detent(levels_[Controls::S36], 0.05f);
    const bool  input_centre = input == 0.5f;
    p.input                  = kMaxInput * input * input;
    const float output       = levels_[Controls::S37];
    p.output                 = output * output;

    core.SetParams(p);

    // Reaching a detent, or turning within it: a pulse
    const std::array<bool, kNumDetents>  at    = {unison,
                                              speed_centre,
                                              size_centre,
                                              tone_centre,
                                              shape_centre,
                                              input_centre};
    const std::array<float, kNumDetents> knobs = {main_[Controls::S33],
                                                  main_[Controls::S36],
                                                  main_[Controls::S31],
                                                  shift_[Controls::S35],
                                                  shift_[Controls::S31],
                                                  levels_[Controls::S36]};
    if(!detents_ready_)
    {
        // Power-up: no pulse
        at_detent_     = at;
        detent_knob_   = knobs;
        detents_ready_ = true;
    }
    for(size_t i = 0; i < kNumDetents; i++)
    {
        const bool moved = std::fabs(knobs[i] - detent_knob_[i]) > kPulseStep;
        if(at[i] && (!at_detent_[i] || moved))
            pulse_until_ = c.now_ms + kPulseMs;
        if(!at[i] || !at_detent_[i] || moved)
            detent_knob_[i] = knobs[i];
        at_detent_[i] = at[i];
    }
}

// First that applies: recording flash, detent pulse, second layer (steady,
// brightness = value last turned), grains (dim glow that follows their windows)
float CoreUI::Led(uint32_t now, const Core& core) const
{
    float brightness;
    if(recording_)
    {
        // From the touch: a tap shows one flash
        brightness
            = ((now - record_since_) / kRecordFlashMs) % 2 == 0 ? 1.0f : 0.0f;
    }
    else if(now < pulse_until_)
    {
        brightness = 1.0f;
    }
    else if(shifted_)
    {
        brightness = shift_[shift_shown_.knob];
    }
    else if(levels_on_)
    {
        brightness = levels_[levels_shown_.knob];
    }
    else
    {
        brightness = kGlow * std::min(1.0f, core.GrainLevel());
    }
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
