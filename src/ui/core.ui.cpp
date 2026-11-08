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

    // Of each side of S37's centre, 0..1: none within the detent
    float Side(float density)
    {
        return std::clamp(
            (std::fabs(2.0f * density - 1.0f) - 2.0f * CoreUI::kDensityDetent)
                / (1.0f - 2.0f * CoreUI::kDensityDetent),
            0.0f,
            1.0f);
    }

    // Either side of the centre, the stops in turn: flat for the first third
    // of each segment, smooth between
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

    // Semitones, +-24: unison, minor and major third, fifth, octaves
    constexpr float kIntervals[] = {0.0f, 1.0f, 3.0f, 4.0f, 7.0f, 12.0f, 24.0f};
    // Speed either way: held at the centre, stretched to a quarter and a half,
    // real time, two and four times
    constexpr float kSpeeds[] = {0.0f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f};

    // S32, Spray, 0..1: how far each grain strays from the playhead, evenly:
    // position within the amount around it, then size, pitch, pan, level and
    // reverb send
    void Spray(float s, Core::Params& p)
    {
        p.rand_pos   = s * s;
        p.rand_size  = s * s / 4.0f;
        p.rand_pitch = s * s * s;
        p.rand_gain  = s;
        p.rand_send  = s;
        p.spread     = s;
    }

    float Reverb(Controls::Position s09)
    {
        return s09 == Controls::Up
                   ? CoreUI::kReverbLong
                   : s09 == Controls::Centre ? CoreUI::kReverbShort : 0.0f;
    }

    End EndOf(Controls::Position s07)
    {
        return s07 == Controls::Up
                   ? End::Once
                   : s07 == Controls::Centre ? End::Turn : End::Loop;
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

    // S36, Speed: held at the centre, forward up, reverse down: the time
    // stretched, the grains' pitch their own. P02 or a place held: held too.
    // S07, End: once, turn, loop.
    const float speed = Stops(c.knobs[Controls::S36], kSpeeds);
    const bool  place = Pads(c, core);
    p.speed           = speed;
    p.freeze          = speed == 0.0f || place || c.touched[kHoldPad];
    p.end             = EndOf(c.switches[Controls::S07]);

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

    // S37, Density: grains at once, so per second at this size. Below the
    // centre regular, above random.
    const float side = Side(c.knobs[Controls::S37]);
    const float overlap
        = kMinOverlap * std::pow(kMaxOverlap / kMinOverlap, side);
    p.density       = side > 0.0f ? overlap * 1000.0f / p.grain_ms : 0.0f;
    p.random_starts = c.knobs[Controls::S37] > 0.5f;

    // S34, Shape: the grain's window, reverse percussive, Hann at the centre,
    // percussive
    const float shape = Detent(c.knobs[Controls::S34], 0.05f);
    p.window_shape    = shape;

    // S33, Pitch
    const float semitones = Stops(c.knobs[Controls::S33], kIntervals);
    p.pitch               = std::exp2(semitones / 12.0f);

    // S09, Reverb. P01 held: mute. P00 held: Spray at its top.
    p.reverb = Reverb(c.switches[Controls::S09]);
    p.mute   = c.touched[kMutePad];
    Spray(c.touched[kShakePad] ? 1.0f : c.knobs[Controls::S32], p);

    core.SetParams(p);
    Pulse(c,
          {semitones == 0.0f,
           std::fabs(speed) == 1.0f,
           size == 0.5f,
           shape == 0.5f});
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

// P11: the glass turned over. P03-P09: the selection in sevenths, left to
// right. A touch jumps the playhead to its place with a grain at once, the
// starts counted from it.
bool CoreUI::Pads(const Controls& c, Core& core)
{
    const bool turn = c.touched[kTurnPad];
    if(turn && !turn_touched_)
        core.Turn();
    turn_touched_ = turn;

    bool held = false;
    for(size_t i = 0; i < kNumPlaces; i++)
    {
        const bool touched = c.touched[kPlacePads[i]];
        if(touched && !places_touched_[i])
            core.Jump(float(i) / float(kNumPlaces));
        places_touched_[i] = touched;
        held               = held || touched;
    }
    return held;
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
