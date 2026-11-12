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


    End EndOf(Controls::Position s07)
    {
        return s07 == Controls::Up
                   ? End::Loop
                   : s07 == Controls::Centre ? End::Turn : End::Once;
    }

    // Off at the bottom, then kVolumeDb up to 0 dB
    float Volume(float x)
    {
        return x > 0.0f
                   ? std::pow(10.0f, CoreUI::kVolumeDb * (x - 1.0f) / 20.0f)
                   : 0.0f;
    }

} // namespace

CoreUI::CoreUI()
{
    twin_[Controls::S30] = Takeover(kFilter, true);
    twin_[Controls::S31] = Takeover(kRandomSize);
    twin_[Controls::S32] = Takeover(kRandomTime);
    twin_[Controls::S33] = Takeover(kRandomPitch, true);
    twin_[Controls::S34] = Takeover(kRandomShape);
    twin_[Controls::S35] = Takeover(kReverb);
    twin_[Controls::S36] = Takeover(kReverse);
    twin_[Controls::S37] = Takeover(kVolume);
}

void CoreUI::Process(const Controls& c, Core& core)
{
    // S09, Direction: forward up, frozen at the centre, reverse down
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
    // end. Both always alive. The playhead keeps its share of the loop, so
    // Position carries it through the recording, playing or frozen. With
    // Shift, S30: the filter, low-pass down, off at the centre, high-pass up.
    // S35: the reverb.
    p.start = std::min(own(Controls::S30), 1.0f - Core::kMinLength);
    p.length
        = std::max(own(Controls::S35) * (1.0f - p.start), Core::kMinLength);
    const float filter = Detent(twin(Controls::S30), 0.05f);
    p.filter           = 2.0f * filter - 1.0f;
    p.reverb           = twin(Controls::S35);

    // S36, Speed: the time stretched, the grains' pitch their own. Frozen, it
    // scrubs through the loop instead, and takes the speed up again after.
    // Shift, Reverse: the odds a grain reads the other way. S07, End: at the
    // loop's end once and no more, turned over, or round again.
    const float speed = Stops(0.5f + 0.5f * own(Controls::S36), kSpeeds);
    Slices(c, core, p);
    p.speed   = mode == Controls::Down ? -speed : speed;
    p.freeze  = frozen || speed == 0.0f;
    p.end     = EndOf(c.switches[Controls::S07]);
    p.reverse = twin(Controls::S36);

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

    // S37, Density: grains per second, none at the bottom. Shift: the output
    // volume.
    const float rate
        = (own(Controls::S37) - kDensityNone) / (1.0f - kDensityNone);
    p.density = rate > 0.0f
                    ? kMinDensity * std::pow(kMaxDensity / kMinDensity, rate)
                    : 0.0f;
    p.volume = Volume(twin(Controls::S37));

    // S32, Spray: how far each grain strays from the playhead,
    // evenly, up to the whole loop, and across the stereo field with it.
    // Shift: the grains' starts, from regular to random.
    const float rnd_pos = own(Controls::S32);
    p.rand_pos          = rnd_pos * rnd_pos;
    p.spread            = rnd_pos;
    p.rand_time         = twin(Controls::S32);

    // S33, Pitch. Shift: its randomness, none at the centre. Up: free, to an
    // octave either way. Down: in octaves, more grains an octave away.
    const float semitones = Stops(own(Controls::S33), kIntervals);
    const float rnd_pitch = 2.0f * Detent(twin(Controls::S33), 0.05f) - 1.0f;
    p.pitch               = std::exp2(semitones / 12.0f);
    p.rand_pitch = rnd_pitch < 0.0f ? rnd_pitch : rnd_pitch * rnd_pitch;

    // S34, Shape: the grain's window, square, decay, bell and reverse at
    // even steps along the knob. Shift: its randomness.
    p.window_shape = own(Controls::S34);
    p.rand_shape   = twin(Controls::S34);

    const bool trig = Transport(c, p);
    core.SetParams(p);
    if(trig)
        core.Jump(p.speed < 0.0f ? 1.0f : 0.0f);
    Pulse(c,
          {semitones == 0.0f,
           !frozen && speed == 1.0f,
           size == 0.5f,
           filter == 0.5f,
           rnd_pitch == 0.0f});
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
        Takeover& was = shift_ ? twin_[k] : own_[k];
        Takeover& now = shift ? twin_[k] : own_[k];
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

// P00, held: stopped as a tape machine, everything slowing to a halt, back
// to speed on release. P02, Trigger: playback from the loop's start, its end in
// reverse, with a grain at once.
bool CoreUI::Transport(const Controls& c, Core::Params& p)
{
    const bool trig = c.touched[kTrigPad] && !trig_touched_;
    trig_touched_   = c.touched[kTrigPad];
    p.stop          = c.touched[kStopPad];
    return trig;
}

// P01: the second layer's values back to their defaults
void CoreUI::Reset(const Controls& c)
{
    const bool touched = c.touched[kResetPad];
    if(touched && !reset_touched_)
    {
        for(size_t k = 0; k < Controls::kNumKnobs; k++)
            twin_[k].Set(kTwinDefaults[k], c.knobs[k]);
    }
    reset_touched_ = touched;
}

void CoreUI::Default(Controls::Knob k, bool twin, float knob)
{
    if(twin)
        twin_[k].Set(kTwinDefaults[k], knob);
    else
        own_[k].Set(knob, knob);
}

// P10: record while held, the grains playing on. A tap keeps the seconds
// before it. Touched with Shift: the input heard with the grains, nothing
// recorded.
void CoreUI::Record(const Controls& c, Core& core, Core::Params& p)
{
    const bool touched = c.touched[kRecordPad];
    if(touched && !record_touched_)
        listening_ = shift_;
    record_touched_ = touched;
    record_         = touched && !listening_;
    core.SetRecording(record_);
    p.mix = touched && listening_ ? kMonitorMix : 1.0f;
}

// P03-P09: the loop in slices, left to right, each from an onset of the
// recording. Each held one plays its slice. A new recording ends those held.
void CoreUI::Slices(const Controls& c, const Core& core, Core::Params& p)
{
    const bool fresh = core.Recordings() != analysed_;
    if(fresh)
    {
        analysed_           = core.Recordings();
        const Buffer& sound = core.Sound();
        onsets_.Begin(sound.Samples(), sound.Length(), float(kSampleRateHz));
    }
    onsets_.Step(kOnsetFrames);
    slice_starts_ = onsets_.Slices(p.start, p.length);
    for(size_t i = 0; i < kNumSlices; i++)
    {
        const bool touched = c.touched[kSlicePads[i]];
        slices_stale_[i]   = touched && (fresh || slices_stale_[i]);
        const float end    = i + 1 < kNumSlices ? slice_starts_[i + 1] : 1.0f;
        p.slice_start[i]   = slice_starts_[i];
        p.slice_length[i]  = std::max(end - slice_starts_[i], 0.0f);
        if(touched && !slices_stale_[i])
            p.slices_held |= 1u << i;
    }
}

// Reaching a detent, or turning within it: a pulse
void CoreUI::Pulse(const Controls& c, const std::array<bool, kNumDetents>& at)
{
    const std::array<float, kNumDetents> values
        = {own_[Controls::S33].Value(),
           own_[Controls::S36].Value(),
           own_[Controls::S31].Value(),
           twin_[Controls::S30].Value(),
           twin_[Controls::S33].Value()};
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
// follows the grains
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
        brightness = kGlow * std::min(1.0f, core.GrainLevel());
    // Cubed: even steps to the eye
    return brightness * brightness * brightness;
}

} // namespace clessidra
