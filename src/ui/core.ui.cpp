#include "core.ui.h"

#include <algorithm>
#include <cmath>

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

    // 0 below from, 1 above to, smooth between
    float Ramp(float x, float from, float to)
    {
        const float t = std::clamp((x - from) / (to - from), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    // A pad's pressure above a light touch, 0..1
    float Firm(float pressure)
    {
        return std::clamp((pressure - CoreUI::kPressFrom)
                              / (1.0f - CoreUI::kPressFrom),
                          0.0f,
                          1.0f);
    }

    // Grains per second at density, 0..1: none at the bottom, then exponential
    float GrainsPerSecond(float density)
    {
        if(density < CoreUI::kDensityOff)
            return 0.0f;
        const float open
            = (density - CoreUI::kDensityOff) / (1.0f - CoreUI::kDensityOff);
        return CoreUI::kDensityMin
               * std::pow(Core::kMaxDensity / CoreUI::kDensityMin, open);
    }

    // Beads' detune law (cv_reader.h, LAW_DETUNE): either side of the centre,
    // the stops in turn, flat for the first third of each segment, smooth
    // between
    template <size_t N>
    float Stops(float x, const float (&stops)[N])
    {
        constexpr size_t kSegments = N - 1;
        const float      centred   = 2.0f * std::clamp(x, 0.0f, 1.0f) - 1.0f;
        const float      position  = std::fabs(centred) * float(kSegments);
        const size_t     i         = std::min(size_t(position), kSegments - 1);
        const float      t         = std::clamp(
            (position - float(i) - 1.0f / 3.0f) * 1.5f, 0.0f, 1.0f);
        const float smooth = t * t * (3.0f - 2.0f * t);
        const float value  = stops[i] + smooth * (stops[i + 1] - stops[i]);
        return centred < 0.0f ? -value : value;
    }

    // Semitones: Beads' intervals, +-24
    constexpr float kIntervals[] = {0.0f, 1.0f, 3.0f, 4.0f, 7.0f, 12.0f, 24.0f};
    // Speed: stopped at the centre, 1, 2 and 4 times either way
    constexpr float kSpeeds[] = {0.0f, 1.0f, 2.0f, 4.0f};

    // S32, Spray, 0..1: from plain, periodic grains to random starts, then
    // positions, sizes and shapes further from the playhead, wider pan, and
    // over the top quarter random pitch. Each mostly near with a long tail, as
    // Beads' attenurandomizers turned left.
    void Spray(float s, Core::Params& p)
    {
        p.rand_density = -std::min(2.0f * s, 1.0f);
        p.rand_pos     = -s * s * s;
        p.rand_size    = -s * s / 3.0f;
        p.rand_shape   = -s * s / 3.0f;
        p.rand_pitch   = -Ramp(s, CoreUI::kPitchSprayFrom, 1.0f) / 3.0f;
        p.rand_gain    = -s * s / 3.0f;
        p.rand_send    = -s * s / 3.0f;
        p.spread       = s;
    }

} // namespace

CoreUI::CoreUI() = default;

void CoreUI::Process(const Controls& c, Core& core)
{
    Core::Params p;
    Record(c, core, p);

    // S30, Position: where the sand starts, anywhere in the recording, round
    // the end. S35, Length: how much sand.
    p.start  = std::min(c.knobs[Controls::S30], 0.999f);
    p.length = std::max(c.knobs[Controls::S35], Core::kMinLength);

    // S36, Speed: stopped at the centre, forward up, reverse down, at 1, 2
    // and 4 times. S07, End: once, turn, loop. S09, Sand: hold, wear, live.
    const float              speed = Stops(c.knobs[Controls::S36], kSpeeds);
    const Controls::Position end   = c.switches[Controls::S07];
    p.end                          = end == Controls::Up
                ? End::Once
                : end == Controls::Centre ? End::Turn : End::Loop;
    const Controls::Position sand = c.switches[Controls::S09];
    p.sand                        = sand == Controls::Up ? Core::Sand::Hold
                                  : sand == Controls::Centre ? Core::Sand::Wear
                                                             : Core::Sand::Live;

    // S37, Density: grains per second, the reverb following, more for a lone
    // grain
    const float density = c.knobs[Controls::S37];
    p.density           = GrainsPerSecond(density);
    p.reverb = kReverbSparse + density * (kReverbDense - kReverbSparse);

    // S31, Size: kMinGrainMs to kMaxGrainMs, exponential, 100 ms at the
    // centre, up to half the recording
    const float seconds = float(core.SoundSamples()) / float(kSampleRateHz);
    const float size    = Detent(c.knobs[Controls::S31], 0.05f);
    const float top_ms
        = seconds > 0.0f
              ? std::clamp(500.0f * seconds, kMinGrainMs, kMaxGrainMs)
              : kMaxGrainMs;
    p.grain_ms = std::min(
        kMinGrainMs * std::pow(kMaxGrainMs / kMinGrainMs, size), top_ms);
    p.min_grain_ms = kMinGrainMs;
    p.max_grain_ms = top_ms;

    // S34, Shape: the grain's window, reverse percussive, Hann at the centre,
    // percussive
    const float shape = Detent(c.knobs[Controls::S34], 0.05f);
    p.window_shape    = shape;

    // S33, Pitch
    const float semitones = Stops(c.knobs[Controls::S33], kIntervals);
    p.pitch               = std::exp2(semitones / 12.0f);

    // P01 held: mute
    p.mute = c.touched[kMutePad];

    // The gestures bend all of it while held, S32's Spray among it
    p.speed = speed;
    Gestures(c, c.knobs[Controls::S32], p);
    core.SetParams(p);
    Pads(c, core);
    Pulse(c,
          {semitones == 0.0f,
           std::fabs(speed) == 1.0f,
           size == 0.5f,
           shape == 0.5f});
}

// P03-P09, P11: held, each bends the glass, pressure further, and settles back
// on release: towards no grains, the most grains, the finest, the most spray,
// stopped, the fastest, the farthest, reversed. Then Spray and the playhead.
void CoreUI::Gestures(const Controls& c, float spray, Core::Params& p)
{
    const uint32_t dt = c.now_ms - gesture_ms_;
    gesture_ms_       = c.now_ms;
    std::array<float, kNumGestures> d{};
    for(size_t g = 0; g < kNumGestures; g++)
    {
        const size_t pad    = kGesturePads[g];
        const float  target = c.touched[pad] ? kGestureLight
                                                  + (1.0f - kGestureLight)
                                                        * Firm(c.pressure[pad])
                                            : 0.0f;
        const float ms
            = target > depth_[g] ? kGestureAttackMs : kGestureReleaseMs;
        depth_[g] += (target - depth_[g]) * (1.0f - std::exp(-float(dt) / ms));
        if(std::fabs(target - depth_[g]) < 5e-3f)
            depth_[g] = target;
        d[g] = depth_[g];
    }

    p.density *= 1.0f - d[Clog];
    p.density += d[Burst] * (Core::kMaxDensity - p.density);
    p.grain_ms *= std::pow(kFineMs / p.grain_ms, d[Fine]);
    spray += d[Shake] * (1.0f - spray);
    p.reverb += d[Far] * (1.0f - p.reverb);

    float       speed = p.speed * (1.0f - d[Drag]);
    const float way   = speed < 0.0f ? -1.0f : 1.0f;
    speed
        += way * d[Rush] * (kSpeeds[std::size(kSpeeds) - 1] - std::fabs(speed));
    // Through zero, as a tape run back
    speed *= 1.0f - 2.0f * d[Reverse];
    p.speed  = speed;
    p.freeze = std::fabs(speed) < kStopped;

    Spray(spray, p);
}

// P10: record while held. A tap keeps the seconds before it, held longer the
// input is heard with the grains.
void CoreUI::Record(const Controls& c, Core& core, Core::Params& p)
{
    const bool rec = c.touched[kRecordPad];
    if(rec && !record_)
        rec_since_ = c.now_ms;
    record_ = rec;
    core.SetRecording(rec);
    const bool holding
        = rec && c.now_ms - rec_since_ >= uint32_t(Core::kTapSeconds * 1000.0f);
    p.mix = holding ? kMonitorMix : 1.0f;
}

// P00: Seed, one grain at the playhead, as Beads' button. P02: the playhead
// back to the start.
void CoreUI::Pads(const Controls& c, Core& core)
{
    if(c.touched[kSeedPad] && !seed_touched_)
        core.Seed();
    seed_touched_ = c.touched[kSeedPad];
    if(c.touched[kRestartPad] && !restart_touched_)
        core.Restart();
    restart_touched_ = c.touched[kRestartPad];
}

// Reaching a detent, or turning within it: a pulse
void CoreUI::Pulse(const Controls& c, const std::array<bool, kNumDetents>& at)
{
    const std::array<float, kNumDetents> knobs = {c.knobs[Controls::S33],
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

// Flashing while recording, the detent pulse, or a glow that follows the
// grains, dimming as the sand runs out
float CoreUI::Led(uint32_t now, const Core& core) const
{
    if(record_)
        return (now / kFlashMs) % 2 == 0 ? 1.0f : 0.0f;
    const float sand = kSandFloor + (1.0f - kSandFloor) * core.SandLeft();
    const float brightness
        = now < pulse_until_ ? 1.0f
                             : kGlow * sand * std::min(1.0f, core.GrainLevel());
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
