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

    // Grains per second at amount 0..1, exponential
    constexpr float kDensityMin = 0.3f;

    float Rate(float amount)
    {
        return kDensityMin * std::pow(Core::kMaxDensity / kDensityMin, amount);
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
    // P11: the randomness of position, size, shape and pitch. P00: filter and
    // reverb, in the order the sound goes through them.
    for(auto k : {Controls::S30, Controls::S31, Controls::S32, Controls::S33})
        layers_.Add(kRandom, k, kRandomStart);
    layers_.Add(kFx, Controls::S31, kFilterStart);
    layers_.Add(kFx, Controls::S32, kReverbStart);
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

    // P00 held: the FX layer, else P11 held: the randomness
    layer_ = c.touched[kFxPad] ? kFx : c.touched[kRandomPad] ? kRandom : kOwn;
    layers_.Process(c.knobs, layer_);
    const auto knob
        = [this](Controls::Knob k) { return layers_.Value(kOwn, k); };

    // P10: record while held. A new recording: its slices.
    record_ = c.touched[kRecordPad];
    core.SetRecording(record_);
    if(core.Recordings() != recordings_)
    {
        recordings_         = core.Recordings();
        const Buffer& sound = core.Sound();
        slices_ = FindSlices(sound.Data(), sound.Length(), kSampleRateHz);
    }

    // S35, length of the selection. S30, its position: from the start to the
    // end of the recording.
    const float seconds = float(core.SoundSamples()) / float(kSampleRateHz);
    p.length            = std::max(knob(Controls::S35), kMinLength);
    p.start             = knob(Controls::S30) * (1.0f - p.length);

    const float semitones = Semitones(knob(Controls::S33));
    const bool  unison    = semitones == 0.0f;
    p.pitch               = std::exp2(semitones / 12.0f);

    // S34: dry and wet
    p.mix = knob(Controls::S34);

    // P00 + S31: filter, low-pass down, high-pass up, off at the centre.
    // P00 + S32: reverb.
    p.filter = 2.0f * Detent(layers_.Value(kFx, Controls::S31), 0.05f) - 1.0f;
    p.reverb = layers_.Value(kFx, Controls::S32);

    // S36, speed: 0 to 4 times, 1 at the centre, with a detent. S09,
    // direction: up forward, centre freeze, down reverse.
    const float              speed     = Detent(knob(Controls::S36), 0.05f);
    const bool               speed_one = speed == 0.5f;
    const Controls::Position way       = c.switches[Controls::S09];
    p.freeze = way == Controls::Centre || speed == 0.0f;
    p.speed  = (way == Controls::Down ? -4.0f : 4.0f) * speed * speed;

    // S37, Density, as Beads: off at the centre, periodic grains below it,
    // random above, more towards the ends. The triggers start theirs alike.
    const float density     = knob(Controls::S37);
    const float from_centre = std::fabs(density - 0.5f);
    const bool  density_off = from_centre <= kDensityStop;
    p.density
        = density_off
              ? 0.0f
              : Rate((from_centre - kDensityStop) / (0.5f - kDensityStop));
    p.jitter = density > 0.5f ? 1.0f : 0.0f;

    // P02: the playhead from the start of the selection
    if(c.touched[kRestartPad] && !restart_touched_)
        core.Restart();
    restart_touched_ = c.touched[kRestartPad];

    // S07, trigger mode: up once, centre while held, down round the slice
    const Controls::Position mode         = c.switches[Controls::S07];
    const auto               trigger_mode = mode == Controls::Up
                                  ? Core::Trigger::Mode::Once
                                  : mode == Controls::Centre
                                        ? Core::Trigger::Mode::Gate
                                        : Core::Trigger::Mode::Loop;

    // P03-P09: each touch plays its slice, its own stream of grains, as S07
    // sets. Pressure sets its density.
    for(size_t i = 0; i < kNumSlices; i++)
    {
        const size_t   pad     = kSlicePads[i];
        Core::Trigger& trigger = triggers_[i];
        if(c.touched[pad] && !slice_touched_[i])
        {
            trigger.id++;
            trigger.start = slices_[i];
            trigger.end   = slices_[i + 1];
        }
        slice_touched_[i] = c.touched[pad];
        trigger.mode      = trigger_mode;
        trigger.held      = c.touched[pad];
        if(c.touched[pad])
        {
            const float firm = std::clamp((c.pressure[pad] - kPressFrom)
                                              / (1.0f - kPressFrom),
                                          0.0f,
                                          1.0f);
            trigger.density = Rate(kTriggerFrom + (1.0f - kTriggerFrom) * firm);
        }
    }
    p.triggers = triggers_;

    // S31: grain size, exponential, kMinGrainMs to kMaxGrainMs, at most half
    // the sound
    const float size_knob   = Detent(knob(Controls::S31), 0.05f);
    const bool  size_centre = size_knob == 0.5f;
    const float top_ms      = std::clamp(500.0f * seconds, 100.0f, kMaxGrainMs);
    p.grain_ms              = std::clamp(
        kMinGrainMs * std::pow(kMaxGrainMs / kMinGrainMs, size_knob),
        kMinGrainMs,
        top_ms);
    p.min_grain_ms = kMinGrainMs;
    p.max_grain_ms = top_ms;

    // S32: window shape
    p.window_shape = knob(Controls::S32);

    // P11 + S30 to S33: the randomness of position, size, shape and pitch, as
    // Beads' attenuverters: off at the centre, an even spread up, mostly near
    // with a long tail down
    const auto random = [this](Controls::Knob k) {
        return 2.0f * Detent(layers_.Value(kRandom, k), 0.05f) - 1.0f;
    };
    p.rand_pos   = random(Controls::S30);
    p.rand_size  = random(Controls::S31);
    p.rand_shape = random(Controls::S32);
    p.rand_pitch = random(Controls::S33);

    // Pan, as Beads: as wide as the grains differ, in every way
    p.spread = std::min(std::fabs(p.rand_pos) + std::fabs(p.rand_size)
                            + std::fabs(p.rand_shape) + std::fabs(p.rand_pitch)
                            + p.jitter,
                        1.0f);

    core.SetParams(p);


    // Reaching a detent, or turning within it: a pulse
    const std::array<bool, kNumDetents> at
        = {unison, speed_one, size_centre, density_off};
    const std::array<float, kNumDetents> knobs = {knob(Controls::S33),
                                                  knob(Controls::S36),
                                                  knob(Controls::S31),
                                                  knob(Controls::S37)};
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
        = layer_ != kOwn && layers_.Turned()
              ? layers_.TurnedValue()
              : now < pulse_until_ ? 1.0f
                                   : kGlow * std::min(1.0f, core.GrainLevel());
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
