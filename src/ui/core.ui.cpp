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

    // Grains per second at a flow, 0..1: none while closed, then exponential
    float Density(float flow)
    {
        if(flow < CoreUI::kFlowOff)
            return 0.0f;
        const float open
            = (flow - CoreUI::kFlowOff) / (1.0f - CoreUI::kFlowOff);
        return CoreUI::kDensityMin
               * std::pow(Core::kMaxDensity / CoreUI::kDensityMin, open);
    }

    // +-12 semitones with stops at unison, a detune, the fourth, the fifth and
    // the octave: flat for the first third of each segment, smooth between
    float Semitones(float x)
    {
        static constexpr float kIntervals[] = {0.0f, 0.1f, 5.0f, 7.0f, 12.0f};
        constexpr size_t       kSegments    = std::size(kIntervals) - 1;
        const float  centred  = 2.0f * std::clamp(x, 0.0f, 1.0f) - 1.0f;
        const float  position = std::fabs(centred) * float(kSegments);
        const size_t i        = std::min(size_t(position), kSegments - 1);
        const float  t        = std::clamp(
            (position - float(i) - 1.0f / 3.0f) * 1.5f, 0.0f, 1.0f);
        const float smooth = t * t * (3.0f - 2.0f * t);
        const float value
            = kIntervals[i] + smooth * (kIntervals[i + 1] - kIntervals[i]);
        return centred < 0.0f ? -value : value;
    }

    // S32, Scatter, 0..1: from plain, periodic grains to random starts, then
    // positions, sizes and shapes further from the playhead, wider pan, and
    // over the top quarter random pitch. Each mostly near with a long tail, as
    // sand falls.
    void Scatter(float s, Core::Params& p)
    {
        p.rand_density = -std::min(2.0f * s, 1.0f);
        p.rand_pos     = -s * s * s;
        p.rand_size    = -s * s / 3.0f;
        p.rand_shape   = -s * s / 3.0f;
        p.rand_pitch   = -Ramp(s, CoreUI::kPitchScatterFrom, 1.0f) / 3.0f;
        p.spread       = s;
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
    Record(c, core, p);

    // S30, Position: where the sand starts, anywhere in the recording, round
    // the end. S35, Length: how much sand.
    p.start  = std::min(c.knobs[Controls::S30], 0.999f);
    p.length = std::max(c.knobs[Controls::S35], Core::kMinLength);

    // S36, Speed: how fast the glass empties, stopped at the bottom, 1 at the
    // centre, 4 at the top. S09: forward, held, reverse; flicked, the glass
    // turned. S07, End: once, turn over, loop.
    const float              speed = Detent(c.knobs[Controls::S36], 0.05f);
    const Controls::Position way   = c.switches[Controls::S09];
    p.freeze                       = way == Controls::Centre;
    p.speed = (way == Controls::Down ? -4.0f : 4.0f) * speed * speed;
    const Controls::Position end = c.switches[Controls::S07];
    p.end                        = end == Controls::Up
                ? End::Once
                : end == Controls::Centre ? End::Turn : End::Loop;

    // S37, Flow: the neck. Density, a size, the window from percussive drops to
    // a Hann cloud, and the reverb, more for a lone drop.
    const float flow = c.knobs[Controls::S37];
    p.density        = Density(flow);
    p.window_shape   = 1.0f - 0.5f * Ramp(flow, kShapeFrom, kShapeTo);
    p.reverb         = kReverbDrops + flow * (kReverbCloud - kReverbDrops);

    // S31, Size: two octaves either way of Flow's, up to half the recording
    const float seconds = float(core.SoundSamples()) / float(kSampleRateHz);
    const float size    = Detent(c.knobs[Controls::S31], 0.05f);
    const float flow_ms
        = kFlowGrainMsLo * std::pow(kFlowGrainMsHi / kFlowGrainMsLo, flow);
    const float top_ms
        = seconds > 0.0f
              ? std::clamp(500.0f * seconds, kMinGrainMs, kMaxGrainMs)
              : kMaxGrainMs;
    p.grain_ms = std::clamp(
        flow_ms * std::exp2(4.0f * size - 2.0f), kMinGrainMs, top_ms);
    p.min_grain_ms = kMinGrainMs;
    p.max_grain_ms = top_ms;

    // S32, Scatter, at least Flow's floor; P11 held shakes it towards the top
    float scatter = std::max(c.knobs[Controls::S32],
                             kCloudScatter * Ramp(flow, kCloudFrom, 1.0f));
    if(c.touched[kShakePad])
        scatter += (1.0f - scatter)
                   * (kShakeFrom
                      + (1.0f - kShakeFrom) * Firm(c.pressure[kShakePad]));
    Scatter(scatter, p);

    // S33, Pitch. S34, Tone: low-pass down, off at the centre, high-pass up.
    const float semitones = Semitones(c.knobs[Controls::S33]);
    p.pitch               = std::exp2(semitones / 12.0f);
    const float tone      = Detent(c.knobs[Controls::S34], 0.05f);
    p.filter              = 2.0f * tone - 1.0f;

    // P00 held: mute
    p.mute = c.touched[kMutePad];

    Triggers(c, flow, p);
    core.SetParams(p);
    Pads(c, core);
    Pulse(c, {semitones == 0.0f, speed == 0.5f, size == 0.5f, tone == 0.5f});
}

// P10: record while held. A tap keeps the seconds before it; held longer, the
// input is heard with the grains. A new recording: its slices.
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

    if(core.Recordings() == recordings_)
        return;
    recordings_         = core.Recordings();
    const Buffer& sound = core.Sound();
    slices_ = FindSlices(sound.Data(), sound.Length(), kSampleRateHz);
}

// P01: a drop, one grain at the playhead. P02: the sand back to the top.
void CoreUI::Pads(const Controls& c, Core& core)
{
    if(c.touched[kDropPad] && !drop_touched_)
        core.Drop();
    drop_touched_ = c.touched[kDropPad];
    if(c.touched[kRestartPad] && !restart_touched_)
        core.Restart();
    restart_touched_ = c.touched[kRestartPad];
}

// P03-P09: each touch plays its slice, its own stream of grains, as End sets:
// once to its end, turned over or round while held. The fader's flow, at
// least half open, opened further by pressure.
void CoreUI::Triggers(const Controls& c, float flow, Core::Params& p)
{
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
        trigger.mode      = p.end;
        trigger.held      = c.touched[pad];
        if(!c.touched[pad])
            continue;
        const float open = std::max(flow, kSliceFlow);
        trigger.density = Density(open + (1.0f - open) * Firm(c.pressure[pad]));
    }
    p.triggers = triggers_;
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
