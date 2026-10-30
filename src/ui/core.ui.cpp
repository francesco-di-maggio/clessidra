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

    // S37: off at the bottom of travel, then 0.3 .. kMaxDensity grains per
    // second, exponential
    constexpr float kDensityOff = 0.02f;
    constexpr float kDensityMin = 0.3f;

    float Density(float x)
    {
        if(x < kDensityOff)
            return 0.0f;
        return kDensityMin
               * std::pow(Core::kMaxDensity / kDensityMin,
                          (x - kDensityOff) / (1.0f - kDensityOff));
    }

    // 10 ms .. top_ms, exponential in each half: centre = 100 ms
    float GrainMs(float x, float top_ms)
    {
        x = Detent(x, 0.05f);
        return x < 0.5f ? 10.0f * std::pow(10.0f, 2.0f * x)
                        : 100.0f * std::pow(top_ms / 100.0f, 2.0f * x - 1.0f);
    }

    // +-24 semitones with stops at musical intervals, flat for the first third
    // of each segment, smooth between them
    float Semitones(float x)
    {
        static constexpr float kIntervals[]
            = {0.0f, 0.1f, 1.0f, 3.0f, 4.0f, 7.0f, 12.0f, 24.0f, 24.0f};
        const float  centred  = 2.0f * std::clamp(x, 0.0f, 1.0f) - 1.0f;
        const float  position = std::fabs(centred) * 7.0f;
        const size_t i        = size_t(position);
        const float  t
            = std::max((position - float(i) - 1.0f / 3.0f) * 1.5f, 0.0f);
        const float smooth = t * t * (3.0f - 2.0f * t);
        const float value
            = kIntervals[i] + smooth * (kIntervals[i + 1] - kIntervals[i]);
        return centred < 0.0f ? -value : value;
    }

} // namespace

CoreUI::CoreUI()
{
    const Core::Params d;
    effects_.values[kInputKnob]    = Takeover(0.5f); // unity
    effects_.values[kOutputKnob]   = Takeover(1.0f);
    effects_.values[kMixKnob]      = Takeover(d.mix);
    effects_.values[kReverbKnob]   = Takeover(d.reverb);
    effects_.values[kResampleKnob] = Takeover(d.resample);
    effects_.values[kToneKnob]     = Takeover(0.5f + 0.5f * d.tone);
    shown_.Update(effects_);
    shown_.knob = kMixKnob;
    for(size_t i = 0; i < kNumSlices; i++)
        slices_[i] = float(i) / float(kNumSlices);
    slices_[kNumSlices] = 1.0f;
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

    // A new recording: its slices
    const uint32_t recordings = core.RecordingCount();
    if(recordings != recordings_seen_)
    {
        const Buffer& buffer = core.Recording();
        slices_ = FindSlices(buffer.Data(), buffer.Length(), kSampleRateHz);
        recordings_seen_ = recordings;
    }

    if(!started_)
    {
        // The main layer starts where its knobs are, and follows them exactly
        for(size_t i = 0; i < Controls::kNumKnobs; i++)
            main_.values[i] = Takeover(c.knobs[i]);
        started_ = true;
    }
    // P11 held: effects
    const bool effects = c.touched[kEffectsPad];
    Layer*     layer   = effects ? &effects_ : &main_;
    if(layer != layer_)
        layer->Engage(c);
    layer_      = layer;
    effects_on_ = effects;
    layer->Process(c);
    shown_.Update(effects_);

    Core::Params p;
    bool         restart = false;

    // S30, S35: the selection, wrapping past the end of the recording, so
    // position is alive at any length
    const float seconds = core.RecordedSeconds();
    p.length            = std::max(main_[Controls::S35], kMinLength);
    p.start             = main_[Controls::S30];

    // Grain size: up to half the recording, and kMaxGrainMs
    const float top_ms     = std::clamp(500.0f * seconds, 100.0f, kMaxGrainMs);
    p.grain_ms             = GrainMs(main_[Controls::S31], top_ms);
    const bool size_centre = Detent(main_[Controls::S31], 0.05f) == 0.5f;

    // S32: from periodic and plain to random starts, positions and sizes, and
    // above the centre pitch. Position in seconds, cubic: fine near 0.
    const float scatter = main_[Controls::S32];
    p.jitter            = scatter;
    p.size_variation    = scatter;
    p.spread            = scatter;
    const float scatter_seconds
        = kMaxScatterSeconds * scatter * scatter * scatter;
    p.rand_pos
        = seconds > 0.0f
              ? std::min(2.0f * scatter_seconds / (p.length * seconds), 1.0f)
              : 0.0f;
    const float above = std::max(2.0f * scatter - 1.0f, 0.0f);
    p.rand_pitch      = above * above * above;

    const float semitones = Semitones(main_[Controls::S33]);
    const bool  unison    = semitones == 0.0f;
    p.pitch               = std::exp2(semitones / 12.0f);

    // Shape: centre detent Hann
    const float shape        = Detent(main_[Controls::S34], 0.05f);
    const bool  shape_centre = shape == 0.5f;
    p.window_shape           = shape;

    // S36: centre stop, up forward, down backward, further faster. Speed 1 half
    // way, with a detent.
    const float tilt     = 2.0f * main_[Controls::S36] - 1.0f;
    const float amount   = std::fabs(tilt);
    p.freeze             = amount < kStopWidth;
    const float speed    = Detent(amount, 0.05f);
    p.speed              = (tilt < 0.0f ? -4.0f : 4.0f) * speed * speed;
    const bool speed_one = speed == 0.5f;

    // S07: up one-shot, centre ping-pong, down loop
    const Controls::Position end = c.switches[Controls::S07];
    p.end                        = end == Controls::Up
                ? Playhead::End::OneShot
                : end == Controls::Centre ? Playhead::End::PingPong
                                          : Playhead::End::Loop;

    // A touched slice: the playhead moves inside it, from its start. Released,
    // the whole selection again, once a one-shot has finished its slice.
    float press   = 0.0f;
    bool  touched = false;
    for(size_t i = 0; i < kNumSlices; i++)
    {
        const size_t pad = kSlicePads[i];
        if(c.touched[pad] && !slice_touched_[i])
        {
            slice_  = i;
            restart = true;
        }
        slice_touched_[i] = c.touched[pad];
        if(c.touched[pad])
        {
            touched = true;
            press   = std::max(press, c.pressure[pad]);
        }
    }
    const bool finishing = p.end == Playhead::End::OneShot && !core.HasEnded();
    if(recording || (!touched && !finishing))
        slice_ = kNumSlices;
    if(slice_ < kNumSlices)
    {
        p.slice_start = (slices_[slice_] - p.start) / p.length;
        p.slice_end   = (slices_[slice_ + 1] - p.start) / p.length;
    }

    // S37: a touch only starts the slice, a firm press adds density
    const float pour
        = std::clamp((press - kPourFrom) / (1.0f - kPourFrom), 0.0f, 1.0f);
    const float density = main_[Controls::S37];
    p.density           = Density(density + pour * pour * (1.0f - density));

    // Effects
    const float tone         = Detent(effects_[kToneKnob], 0.05f);
    const bool  tone_centre  = tone == 0.5f;
    p.tone                   = 2.0f * tone - 1.0f;
    p.mix                    = effects_[kMixKnob];
    p.reverb                 = effects_[kReverbKnob];
    p.resample               = effects_[kResampleKnob];
    const float input        = Detent(effects_[kInputKnob], 0.05f);
    const bool  input_centre = input == 0.5f;
    p.input                  = kMaxInput * input * input;
    const float output       = effects_[kOutputKnob];
    p.output                 = output * output;

    core.SetParams(p);
    if(restart)
        core.Restart();

    // Reaching a detent, or turning within it: a pulse
    const std::array<bool, kNumDetents>  at    = {unison,
                                              p.freeze,
                                              speed_one,
                                              size_centre,
                                              shape_centre,
                                              tone_centre,
                                              input_centre};
    const std::array<float, kNumDetents> knobs = {main_[Controls::S33],
                                                  main_[Controls::S36],
                                                  main_[Controls::S36],
                                                  main_[Controls::S31],
                                                  main_[Controls::S34],
                                                  effects_[kToneKnob],
                                                  effects_[kInputKnob]};
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

// First that applies: recording flash, detent pulse, effects (steady,
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
    else if(effects_on_)
    {
        brightness = effects_[shown_.knob];
    }
    else
    {
        brightness = kGlow * std::min(1.0f, core.GrainLevel());
    }
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
