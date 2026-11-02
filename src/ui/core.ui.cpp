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

    // S37, Density: off at the bottom, then grains per second, exponential
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
    layers_.Add(Controls::S31, kShapeStart);
    layers_.Add(Controls::S33, 0.0f);
    layers_.Add(Controls::S34, kReverbStart);
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

    // P11 held: the second layer
    layer_ = c.touched[kLayerPad];
    layers_.Process(c.knobs, layer_);
    const auto knob = [this](Controls::Knob k) { return layers_.First(k); };

    // P10: record while held. A new recording: its slices.
    record_ = c.touched[kRecordPad];
    core.SetRecording(record_);
    if(core.Recordings() != recordings_)
    {
        recordings_         = core.Recordings();
        const Buffer& sound = core.Sound();
        slices_ = FindSlices(sound.Data(), sound.Length(), kSampleRateHz);
    }

    // S30, S35: start and end of the selection, in either order
    const float seconds = float(core.SoundSamples()) / float(kSampleRateHz);
    const float from    = knob(Controls::S30);
    const float to      = knob(Controls::S35);
    p.length            = std::max(std::fabs(to - from), kMinLength);
    p.start             = std::min(std::min(from, to), 1.0f - p.length);

    const float semitones = Semitones(knob(Controls::S33));
    const bool  unison    = semitones == 0.0f;
    p.pitch               = std::exp2(semitones / 12.0f);

    // S34: dry and wet. P11 + S34: reverb on the grains.
    p.mix    = knob(Controls::S34);
    p.reverb = layers_.Second(Controls::S34);

    // S36, Tilt: centre stop, up forward, down backward, further faster. Speed
    // 1 half way, with a detent.
    const float tilt     = 2.0f * knob(Controls::S36) - 1.0f;
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
    const bool finishing = p.end == Playhead::End::OneShot && !core.HasEnded();
    if(!touched && !finishing)
        slice_ = kNumSlices;
    if(slice_ < kNumSlices)
    {
        // At the sound's onsets, in selections from the selection's start
        p.slice_start = (slices_[slice_] - p.start) / p.length;
        p.slice_end   = (slices_[slice_ + 1] - p.start) / p.length;
    }

    // S37, Density, raised further by a firm press on a slice
    const float firm
        = std::clamp((press - kPressFrom) / (1.0f - kPressFrom), 0.0f, 1.0f);
    const float fader = knob(Controls::S37);
    p.density         = Density(fader + firm * firm * (1.0f - fader));

    // S31: grain size, exponential, kMinGrainMs to kMaxGrainMs, at most half
    // the sound. P11 + S31: window shape.
    const float size_knob   = Detent(knob(Controls::S31), 0.05f);
    const bool  size_centre = size_knob == 0.5f;
    const float top_ms      = std::clamp(500.0f * seconds, 100.0f, kMaxGrainMs);
    p.grain_ms              = std::clamp(
        kMinGrainMs * std::pow(kMaxGrainMs / kMinGrainMs, size_knob),
        kMinGrainMs,
        top_ms);
    p.window_shape = layers_.Second(Controls::S31);

    // S32, Scatter: from periodic and plain to random starts, positions, sizes
    // and pan. Position in seconds, cubic: fine near 0.
    const float scatter = knob(Controls::S32);
    p.jitter            = scatter;
    p.size_variation    = scatter;
    p.spread            = scatter;
    const float scatter_seconds
        = kMaxScatterSeconds * scatter * scatter * scatter;
    p.rand_pos
        = seconds > 0.0f
              ? std::min(2.0f * scatter_seconds / (p.length * seconds), 1.0f)
              : 0.0f;
    // P11 + S33: pitch scatter, cubic: fine near 0
    const float pitch_scatter = layers_.Second(Controls::S33);
    p.rand_pitch              = pitch_scatter * pitch_scatter * pitch_scatter;

    core.SetParams(p);
    if(restart)
        core.Restart();

    // Reaching a detent, or turning within it: a pulse
    const std::array<bool, kNumDetents> at
        = {unison, p.freeze, speed_one, size_centre};
    const std::array<float, kNumDetents> knobs = {knob(Controls::S33),
                                                  knob(Controls::S36),
                                                  knob(Controls::S36),
                                                  knob(Controls::S31)};
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

// Flashing while recording, the detent pulse, or a dim glow that follows the
// grains' windows
float CoreUI::Led(uint32_t now, const Core& core) const
{
    if(record_)
        return (now / kFlashMs) % 2 == 0 ? 1.0f : 0.0f;
    // The second layer: steady at the value of the knob last turned
    const float brightness
        = layer_ && layers_.Turned()
              ? layers_.TurnedValue()
              : now < pulse_until_ ? 1.0f
                                   : kGlow * std::min(1.0f, core.GrainLevel());
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
