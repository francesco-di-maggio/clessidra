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

    // S37, Pour: from closed to single drops, a stream and a cloud. Everything
    // follows from the one gesture.
    constexpr float kPourOff      = 0.02f;
    constexpr float kDensityMin   = 0.3f;
    constexpr float kGrainMinMs   = 30.0f;
    constexpr float kGrainMaxMs   = 300.0f;
    constexpr float kReverbDrops  = 0.7f; // alone in a big glass
    constexpr float kReverbCloud  = 0.15f;
    constexpr float kShapeFrom    = 0.25f; // percussive up to here
    constexpr float kShapeTo      = 0.6f;  // Hann from here
    constexpr float kCloudFrom    = 0.6f;  // grains start to differ from here
    constexpr float kCloudScatter = 0.3f;  // as S32 at this, at the top

    float Density(float pour)
    {
        if(pour < kPourOff)
            return 0.0f;
        return kDensityMin
               * std::pow(Core::kMaxDensity / kDensityMin,
                          (pour - kPourOff) / (1.0f - kPourOff));
    }

    float Ramp(float x, float from, float to)
    {
        const float t = std::clamp((x - from) / (to - from), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
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
    EqualSlices();
}

void CoreUI::EqualSlices()
{
    for(size_t i = 0; i < kNumSlices; i++)
        slices_[i] = float(i) / float(kNumSlices);
    slices_[kNumSlices] = 1.0f;
}

void CoreUI::Process(const Controls& c, Core& core)
{
    Core::Params p;
    bool         restart = false;

    // S30, S35: the selection, from the newest sound held back through the rest,
    // wrapping, so position is alive at any length
    const float seconds = float(core.SoundSamples()) / float(kSampleRateHz);
    p.length            = std::max(c.knobs[Controls::S35], kMinLength);
    p.start             = c.knobs[Controls::S30];

    const float semitones = Semitones(c.knobs[Controls::S33]);
    const bool  unison    = semitones == 0.0f;
    p.pitch               = std::exp2(semitones / 12.0f);

    // S34, Fill: exact at both ends. Sealed again: the sound's slices.
    const float fill_knob = c.knobs[Controls::S34];
    p.fill                = fill_knob < kFillEnd
                 ? 0.0f
                 : fill_knob > 1.0f - kFillEnd ? 1.0f : fill_knob;
    const bool sealed = p.fill == 0.0f;
    if(sealed && !sealed_)
    {
        const Buffer& sound = core.Sound();
        slices_ = FindSlices(sound.Data(), core.SoundSamples(), kSampleRateHz);
    }
    sealed_ = sealed;

    // S36, Tilt: centre stop, up forward, down backward, further faster. Speed
    // 1 half way, with a detent.
    const float tilt     = 2.0f * c.knobs[Controls::S36] - 1.0f;
    const float amount   = std::fabs(tilt);
    p.freeze             = amount < kStopWidth;
    const float speed    = Detent(amount, 0.05f);
    p.speed              = (tilt < 0.0f ? -4.0f : 4.0f) * speed * speed;
    const bool speed_one = speed == 0.5f;

    // S07: up one-shot, centre turn, down loop
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
    const bool finishing
        = p.fill < 1.0f && p.end == Playhead::End::OneShot && !core.HasEnded();
    if(!touched && !finishing)
        slice_ = kNumSlices;
    if(slice_ < kNumSlices)
    {
        // Held: at the sound's onsets, from the selection's start. What comes
        // in: equal parts.
        const float samples = float(std::max(core.SoundSamples(), size_t(1)));
        const float from = float(core.HeldEnd()) / samples - p.start - p.length;
        p.slice_start    = (slices_[slice_] - from) / p.length;
        p.slice_end      = (slices_[slice_ + 1] - from) / p.length;
        p.live_slice_start = float(slice_) / float(kNumSlices);
        p.live_slice_end   = float(slice_ + 1) / float(kNumSlices);
    }

    // S37, Pour, opened further by a firm press on a slice
    const float firm
        = std::clamp((press - kPourFrom) / (1.0f - kPourFrom), 0.0f, 1.0f);
    const float fader = c.knobs[Controls::S37];
    const float pour  = fader + firm * firm * (1.0f - fader);
    p.density         = Density(pour);
    p.reverb          = kReverbDrops + pour * (kReverbCloud - kReverbDrops);
    // 1 percussive .. 0.5 Hann
    p.window_shape = 1.0f - 0.5f * Ramp(pour, kShapeFrom, kShapeTo);

    // S31: grain size on top of what Pour sets, 2 octaves either way
    const float size_knob   = Detent(c.knobs[Controls::S31], 0.05f);
    const bool  size_centre = size_knob == 0.5f;
    const float top_ms      = std::clamp(500.0f * seconds, 100.0f, kMaxGrainMs);
    p.grain_ms
        = std::clamp(kGrainMinMs * std::pow(kGrainMaxMs / kGrainMinMs, pour)
                         * std::exp2(4.0f * size_knob - 2.0f),
                     10.0f,
                     top_ms);

    // S32, Shake: from periodic and plain to random starts, positions and
    // sizes, and above the centre pitch. Position in seconds, cubic: fine near
    // 0. The top of Pour shakes by itself, or the grains would be copies.
    const float scatter = std::max(
        c.knobs[Controls::S32], kCloudScatter * Ramp(pour, kCloudFrom, 1.0f));
    p.jitter         = scatter;
    p.size_variation = scatter;
    p.spread         = scatter;
    const float scatter_seconds
        = kMaxScatterSeconds * scatter * scatter * scatter;
    p.rand_pos
        = seconds > 0.0f
              ? std::min(2.0f * scatter_seconds / (p.length * seconds), 1.0f)
              : 0.0f;
    const float above = std::max(2.0f * scatter - 1.0f, 0.0f);
    p.rand_pitch      = above * above * above;

    core.SetParams(p);
    if(restart)
        core.Restart();

    // Reaching a detent, or turning within it: a pulse
    const std::array<bool, kNumDetents> at
        = {unison, p.freeze, speed_one, size_centre, sealed};
    const std::array<float, kNumDetents> knobs = {c.knobs[Controls::S33],
                                                  c.knobs[Controls::S36],
                                                  c.knobs[Controls::S36],
                                                  c.knobs[Controls::S31],
                                                  c.knobs[Controls::S34]};
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

// The detent pulse, or a dim glow that follows the grains' windows
float CoreUI::Led(uint32_t now, const Core& core) const
{
    const float brightness
        = now < pulse_until_ ? 1.0f : kGlow * std::min(1.0f, core.GrainLevel());
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
