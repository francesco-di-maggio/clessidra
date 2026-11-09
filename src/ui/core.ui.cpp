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
    // Speed: none, stretched to a quarter and a half, real time, two and four
    // times
    constexpr float kSpeeds[] = {0.0f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f};


    Quantize QuantizeOf(Controls::Position s07)
    {
        return s07 == Controls::Up
                   ? Quantize::Octaves
                   : s07 == Controls::Centre ? Quantize::Semitones
                                             : Quantize::Off;
    }

} // namespace

CoreUI::CoreUI()
{
    twin_[Controls::S31] = Takeover(kRandomSize);
    twin_[Controls::S33] = Takeover(kRandomPitch);
    twin_[Controls::S34] = Takeover(kFilter, true);
    twin_[Controls::S36] = Takeover(kDryWet);
    twin_[Controls::S37] = Takeover(kReverb);
}

void CoreUI::Process(const Controls& c, Core& core)
{
    // S09, Mode: forward up, frozen at the centre, reverse down
    const Controls::Position mode   = c.switches[Controls::S09];
    const bool               frozen = mode == Controls::Centre;
    Knobs(c, frozen);
    Scrub(c.knobs[Controls::S36], frozen, core);
    Reset(c);

    Core::Params p;
    Record(c, core, p);

    const auto own  = [this](Controls::Knob k) { return own_[k].Value(); };
    const auto twin = [this](Controls::Knob k) { return twin_[k].Value(); };

    // S30, Position: where the loop starts, anywhere in the recording. S35,
    // Length: a share of what is left after it, at the top to the recording's
    // end. Both always alive, nothing wraps.
    p.start = std::min(own(Controls::S30), 1.0f - Core::kMinLength);
    p.length
        = std::max(own(Controls::S35) * (1.0f - p.start), Core::kMinLength);

    // S36, Speed: the time stretched, the grains' pitch their own. Frozen, it
    // scrubs through the loop instead, and takes the speed up again after. A
    // slice held freezes too.
    const float speed = Stops(0.5f + 0.5f * own(Controls::S36), kSpeeds);
    const bool  slice = Slices(c, core, p.start, p.length);
    p.speed           = mode == Controls::Down ? -speed : speed;
    p.freeze          = frozen || slice || speed == 0.0f;
    p.end             = End::Loop;

    // S31, Size: kMinGrainMs to kMaxGrainMs, exponential, 100 ms at the
    // centre, up to half the recording. Shift: its randomness.
    const float seconds = float(core.SoundSamples()) / float(kSampleRateHz);
    const float size    = Detent(own(Controls::S31), 0.05f);
    const float top_ms
        = seconds > 0.0f
              ? std::clamp(500.0f * seconds, kMinGrainMs, kMaxGrainMs)
              : kMaxGrainMs;
    p.grain_ms = std::min(
        kMinGrainMs * std::pow(kMaxGrainMs / kMinGrainMs, size), top_ms);
    p.min_grain_ms       = kMinGrainMs;
    p.max_grain_ms       = top_ms;
    const float rnd_size = twin(Controls::S31);
    p.rand_size          = rnd_size * rnd_size / 4.0f;

    // S37, Density: grains at once, so per second at this size. Below the
    // centre regular, above random. Shift: the reverb.
    const float side = Side(own(Controls::S37));
    const float overlap
        = kMinOverlap * std::pow(kMaxOverlap / kMinOverlap, side);
    p.density       = side > 0.0f ? overlap * 1000.0f / p.grain_ms : 0.0f;
    p.random_starts = own(Controls::S37) > 0.5f;
    p.reverb        = twin(Controls::S37);

    // S32, Random position: how far each grain strays from the playhead,
    // evenly, up to the whole loop, and across the stereo field with it
    const float rnd_pos = own(Controls::S32);
    p.rand_pos          = rnd_pos * rnd_pos;
    p.spread            = rnd_pos;

    // S33, Pitch. Shift: its randomness, up to an octave either way. S07,
    // Quantize: the random pitch free, in semitones or in octaves.
    const float semitones = Stops(own(Controls::S33), kIntervals);
    const float rnd_pitch = twin(Controls::S33);
    p.pitch               = std::exp2(semitones / 12.0f);
    p.rand_pitch          = rnd_pitch * rnd_pitch;
    p.quantize            = QuantizeOf(c.switches[Controls::S07]);

    // S34, Shape: the grain's window, reverse percussive, Hann at the centre,
    // percussive. Shift: the filter, low-pass down, off at the centre,
    // high-pass up.
    const float shape  = Detent(own(Controls::S34), 0.05f);
    const float filter = Detent(twin(Controls::S34), 0.05f);
    p.window_shape     = shape;
    p.filter           = 2.0f * filter - 1.0f;

    // S36 with Shift: dry to wet, unless P10 asks to hear the input
    if(p.mix == 1.0f)
        p.mix = twin(Controls::S36);

    Transport(c, core, p);
    core.SetParams(p);
    Pulse(c,
          {semitones == 0.0f,
           !frozen && speed == 1.0f,
           size == 0.5f,
           shape == 0.5f,
           filter == 0.5f});
}

// Each knob drives its own value, or its twin while Shift is held. On a layer
// change a knob's last move goes to the layer it leaves, and the other takes
// over from where the knob is.
void CoreUI::Knobs(const Controls& c, bool frozen)
{
    const bool shift = c.touched[kShiftPad];
    for(size_t k = 0; k < Controls::kNumKnobs; k++)
    {
        const float knob = c.knobs[k];
        if(!ready_)
            own_[k] = Takeover(knob, kCentred[k]);
        Takeover& was = shift_ && kTwinned[k] ? twin_[k] : own_[k];
        Takeover& now = shift && kTwinned[k] ? twin_[k] : own_[k];
        // Frozen, S36 scrubs: its speed held, taken up from here after
        const bool speed = k == Controls::S36;
        if(&was != &now)
        {
            if(!(speed && frozen_ && &was == &own_[k]))
                was.Process(knob);
            now.Engage(knob);
            continue;
        }
        if(speed && frozen_ && !frozen && &now == &own_[k])
            now.Engage(knob);
        if(speed && frozen && &now == &own_[k])
            continue;
        const float before = now.Value();
        now.Process(knob);
        if(&now == &twin_[k] && std::fabs(now.Value() - before) > 1e-3f)
            shown_ = now.Value();
    }
    ready_ = true;
    shift_ = shift;
}

// Frozen: S36's moves move the playhead through the loop, a full stroke the
// whole loop. Freezing leaves it where it is.
void CoreUI::Scrub(float fader, bool frozen, Core& core)
{
    if(frozen && !frozen_)
        scrub_from_ = fader;
    frozen_ = frozen;
    if(!frozen || shift_ || std::fabs(fader - scrub_from_) < kScrubStep)
        return;
    core.Scrub(fader - scrub_from_);
    scrub_from_ = fader;
}

// P00: stop as a tape machine, everything slowing to a halt, then a touch
// to start again. P02, Cue: the playhead back to the loop's start, its end
// in reverse, with a grain at once.
void CoreUI::Transport(const Controls& c, Core& core, Core::Params& p)
{
    const bool stop = c.touched[kStopPad];
    if(stop && !stop_touched_)
        stop_ = !stop_;
    stop_touched_ = stop;
    p.stop        = stop_;

    const bool cue = c.touched[kCuePad];
    if(cue && !cue_touched_)
        core.Jump(p.speed < 0.0f ? 1.0f : 0.0f);
    cue_touched_ = cue;
}

// P11 with P01: the second layer's values back to their defaults
void CoreUI::Reset(const Controls& c)
{
    const bool touched = c.touched[kResetPad];
    if(touched && !reset_touched_ && shift_)
    {
        twin_[Controls::S31].Set(kRandomSize, c.knobs[Controls::S31]);
        twin_[Controls::S33].Set(kRandomPitch, c.knobs[Controls::S33]);
        twin_[Controls::S34].Set(kFilter, c.knobs[Controls::S34]);
        twin_[Controls::S36].Set(kDryWet, c.knobs[Controls::S36]);
        twin_[Controls::S37].Set(kReverb, c.knobs[Controls::S37]);
    }
    reset_touched_ = touched;
}

// P10: record while held. A tap keeps the seconds before it, held longer the
// input is heard with the grains. Touched with Shift: the input heard, nothing
// recorded.
void CoreUI::Record(const Controls& c, Core& core, Core::Params& p)
{
    const bool touched = c.touched[kRecordPad];
    if(touched && !record_touched_)
        listening_ = shift_;
    record_touched_ = touched;
    const bool rec  = touched && !listening_;
    if(rec && !record_)
        rec_since_ = c.now_ms;
    record_ = rec;
    core.SetRecording(rec);
    const bool holding
        = rec && c.now_ms - rec_since_ >= uint32_t(Core::kTapSeconds * 1000.0f);
    p.mix = holding || (touched && listening_) ? kMonitorMix : 1.0f;
}

// P03-P09: the loop in slices, left to right, each from an onset of the
// recording. A touch jumps the playhead to its slice with a grain at once, the
// starts counted from it.
bool CoreUI::Slices(const Controls& c, Core& core, float start, float length)
{
    if(core.Recordings() != analysed_)
    {
        analysed_ = core.Recordings();
        onsets_.Find(
            core.Sound().Samples(), core.SoundSamples(), float(kSampleRateHz));
    }
    slice_starts_ = onsets_.Slices(start, length);
    bool held     = false;
    for(size_t i = 0; i < kNumSlices; i++)
    {
        const bool touched = c.touched[kSlicePads[i]];
        if(touched && !slices_touched_[i])
            core.Jump(slice_starts_[i]);
        slices_touched_[i] = touched;
        held               = held || touched;
    }
    return held;
}

// Reaching a detent, or turning within it: a pulse
void CoreUI::Pulse(const Controls& c, const std::array<bool, kNumDetents>& at)
{
    const std::array<float, kNumDetents> values
        = {own_[Controls::S33].Value(),
           own_[Controls::S36].Value(),
           own_[Controls::S31].Value(),
           own_[Controls::S34].Value(),
           twin_[Controls::S34].Value()};
    if(!detents_ready_)
    {
        // Power-up: no pulse
        at_detent_     = at;
        detent_value_  = values;
        detents_ready_ = true;
    }
    for(size_t i = 0; i < kNumDetents; i++)
    {
        const bool moved = std::fabs(values[i] - detent_value_[i]) > kPulseStep;
        if(at[i] && (!at_detent_[i] || moved))
            pulse_until_ = c.now_ms + kPulseMs;
        if(!at[i] || !at_detent_[i] || moved)
            detent_value_[i] = values[i];
        at_detent_[i] = at[i];
    }
}

// Flashing while recording, the detent pulse, Shift's value, or a glow that
// follows the grains, dimming as the sand runs out
float CoreUI::Led(uint32_t now, const Core& core) const
{
    if(record_)
        return (now / kFlashMs) % 2 == 0 ? 1.0f : 0.0f;
    float brightness;
    if(now < pulse_until_)
        brightness = 1.0f;
    else if(shift_)
        brightness = std::max(shown_, kShiftFloor);
    else
    {
        const float sand = kSandFloor + (1.0f - kSandFloor) * core.SandLeft();
        brightness       = kGlow * sand * std::min(1.0f, core.GrainLevel());
    }
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
