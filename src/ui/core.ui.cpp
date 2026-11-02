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
    // Each grain parameter's randomness, then reverb and tone
    layers_.Add(Controls::S30, 0.0f);
    layers_.Add(Controls::S31, 0.0f);
    layers_.Add(Controls::S32, 0.0f);
    layers_.Add(Controls::S33, 0.0f);
    layers_.Add(Controls::S34, kReverbStart);
    layers_.Add(Controls::S35, kToneStart);
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

    // S36, speed: 0 to 4 times, 1 at the centre, with a detent. S09,
    // direction: up forward, centre freeze, down reverse.
    const float              speed     = Detent(knob(Controls::S36), 0.05f);
    const bool               speed_one = speed == 0.5f;
    const Controls::Position way       = c.switches[Controls::S09];
    p.freeze = way == Controls::Centre || speed == 0.0f;
    p.speed  = (way == Controls::Down ? -4.0f : 4.0f) * speed * speed;

    // S37, Density, as Beads: off at the centre, periodic grains below it,
    // random above, more towards the ends. The strikes start theirs alike.
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

    // S07, strike mode: up once, centre while held, down round the slice
    const Controls::Position mode        = c.switches[Controls::S07];
    const auto               strike_mode = mode == Controls::Up
                                 ? Core::Strike::Mode::Once
                                 : mode == Controls::Centre
                                       ? Core::Strike::Mode::Gate
                                       : Core::Strike::Mode::Loop;

    // P03-P09: each touch plays its slice, its own stream of grains, as S07
    // sets. Pressure sets its density.
    for(size_t i = 0; i < kNumSlices; i++)
    {
        const size_t  pad    = kSlicePads[i];
        Core::Strike& strike = strikes_[i];
        if(c.touched[pad] && !slice_touched_[i])
        {
            strike.id++;
            strike.start = slices_[i];
            strike.end   = slices_[i + 1];
        }
        slice_touched_[i] = c.touched[pad];
        strike.mode       = strike_mode;
        strike.held       = c.touched[pad];
        if(c.touched[pad])
        {
            const float firm = std::clamp((c.pressure[pad] - kPressFrom)
                                              / (1.0f - kPressFrom),
                                          0.0f,
                                          1.0f);
            strike.density   = Rate(kStrikeFrom + (1.0f - kStrikeFrom) * firm);
        }
    }
    p.strikes = strikes_;

    // S31: grain size, exponential, kMinGrainMs to kMaxGrainMs, at most half
    // the sound. P11 + S31: its randomness.
    const float size_knob   = Detent(knob(Controls::S31), 0.05f);
    const bool  size_centre = size_knob == 0.5f;
    const float top_ms      = std::clamp(500.0f * seconds, 100.0f, kMaxGrainMs);
    p.grain_ms              = std::clamp(
        kMinGrainMs * std::pow(kMaxGrainMs / kMinGrainMs, size_knob),
        kMinGrainMs,
        top_ms);
    p.rand_size = layers_.Second(Controls::S31);

    // S32: window shape. P11 + S32: its randomness.
    p.window_shape = knob(Controls::S32);
    p.rand_shape   = layers_.Second(Controls::S32);

    // P11 + S30, spray: random positions, in seconds, cubic: fine near 0
    const float spray         = layers_.Second(Controls::S30);
    const float spray_seconds = kMaxSpraySeconds * spray * spray * spray;
    p.rand_pos
        = seconds > 0.0f
              ? std::min(2.0f * spray_seconds / (p.length * seconds), 1.0f)
              : 0.0f;

    // P11 + S33: pitch randomness, cubic: fine near 0
    const float pitch_random = layers_.Second(Controls::S33);
    p.rand_pitch             = pitch_random * pitch_random * pitch_random;

    // Pan, as Beads: as wide as the grains differ, in every way
    p.spread = std::min(
        spray + p.rand_size + p.rand_shape + pitch_random + p.jitter, 1.0f);

    // P11 + S35: tone, low-pass down, high-pass up, off at the centre
    p.tone = 2.0f * Detent(layers_.Second(Controls::S35), 0.05f) - 1.0f;

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
        = layer_ && layers_.Turned()
              ? layers_.TurnedValue()
              : now < pulse_until_ ? 1.0f
                                   : kGlow * std::min(1.0f, core.GrainLevel());
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
