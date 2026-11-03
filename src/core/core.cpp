#include "core.h"

#include <algorithm>
#include <cmath>
#include <Utility/dsp.h>

namespace clessidra
{
void Core::Init(float        sample_rate,
                float*       ring,
                size_t       ring_size,
                float* const buffers[kNumBuffers],
                size_t       buffer_size)
{
    sample_rate_ = sample_rate;
    ring_.Init(ring, ring_size, sample_rate);
    for(size_t i = 0; i < kNumBuffers; i++)
        buffers_[i].Init(buffers[i], buffer_size, sample_rate);
    dc_block_.Init(sample_rate);
    reverb_.Init(sample_rate);
    reverb_.SetLpFreq(kReverbLowpassHz);
    limiter_.Init(sample_rate);

    Grain::InitWindow();
}

void Core::Process(const float* const* in, float** out, size_t size)
{
    const Params  p       = slots_[slot_.load(std::memory_order_acquire)];
    const Buffer& sound   = buffers_[playing_.load(std::memory_order_relaxed)];
    const double  len     = double(sound.Length());
    const double  sel_len = std::max(double(p.length) * len, 1.0);
    const double  sel_start = double(p.start) * len;
    const float   density   = std::clamp(p.density, 0.0f, kMaxDensity);
    // A grain lasts grain_samples whatever its pitch, as the starts per second
    // do: no longer than the recording, and short enough for the voices to hold
    // whole windows at this density. Stolen, they would hardly rise.
    const double most_samples
        = density > 0.0f
              ? double(kNumVoices - kSafetyVoices) * sample_rate_ / density
              : len;
    const double grain_samples
        = std::clamp(p.grain_ms * 0.001 * sample_rate_,
                     1.0,
                     std::max(std::min(len, most_samples), 1.0));
    const double grain_inc = 1.0 / grain_samples;
    // Nothing recorded: no grains, the size as set
    grain_ms_
        = len > 0.0 ? float(1000.0 * grain_samples / sample_rate_) : p.grain_ms;

    // A new length keeps the playhead where it is in the recording. Past the
    // new end: what End does there.
    if(sel_len_ > 0.0 && sel_len != sel_len_)
        stream_.playhead.Scale(sel_len_ / sel_len);
    sel_len_ = sel_len;

    const bool backwards = p.speed < 0.0f;
    if(restart_.exchange(false, std::memory_order_relaxed))
    {
        stream_.playhead.Restart(backwards);
        // With a grain at once
        stream_.sched_phase = stream_.sched_next;
    }
    // A new trigger: from the start of its slice, with a grain at once
    for(size_t s = 0; s < kNumTriggers; s++)
    {
        const Trigger& trigger = p.triggers[s];
        TriggerStream& state   = triggers_[s];
        if(trigger.id == state.id)
            continue;
        state.id = trigger.id;
        state.on = trigger.end > trigger.start && len > 0.0;
        state.playhead.Restart(backwards);
        state.sched_phase = state.sched_next;
    }

    const double   play_inc = p.freeze ? 0.0 : p.speed / sel_len; // per sample
    const uint32_t drops    = drops_.load(std::memory_order_relaxed);
    const bool     drop     = drops != drops_done_ && len > 0.0;
    drops_done_             = drops;

    // Grain starts in this block, at the sample they fall on. A voice that ends
    // in this block is free from the next.
    for(size_t i = 0; i < size && len > 0.0; i++)
    {
        // Through the selection, as End sets at its edges. Run out: no grains
        // but a drop.
        Playhead& playhead = stream_.playhead;
        playhead.Advance(play_inc, p.end);
        const bool due
            = !playhead.Ended()
              && Due(stream_, density / sample_rate_, p.rand_density);
        if(due || (drop && i == 0))
        {
            const bool   reverse = (p.speed < 0.0f) != (playhead.Heading() < 0);
            const Source source
                = {sel_start, sel_len, playhead.Phase(), reverse};
            StartGrain(p, source, &sound, i);
        }

        // Triggers: through their slice at the playhead's speed. Released,
        // only Once goes on, and not while frozen.
        for(size_t s = 0; s < kNumTriggers; s++)
        {
            TriggerStream& state = triggers_[s];
            if(!state.on)
                continue;
            const Trigger& trigger = p.triggers[s];
            if(!trigger.held && (trigger.mode != End::Once || p.freeze))
            {
                state.on = false;
                continue;
            }
            const double slice
                = std::max(double(trigger.end - trigger.start) * len, 1.0);
            state.playhead.Advance(p.freeze ? 0.0 : p.speed / slice,
                                   trigger.mode);
            if(state.playhead.Ended())
            {
                state.on = false;
                continue;
            }
            const float trigger_density
                = std::clamp(trigger.density, 0.0f, kMaxDensity);
            if(!Due(state, trigger_density / sample_rate_, p.rand_density))
                continue;
            const bool reverse
                = (p.speed < 0.0f) != (state.playhead.Heading() < 0);
            const Source source = {double(trigger.start) * len,
                                   slice,
                                   state.playhead.Phase(),
                                   reverse};
            StartGrain(p, source, &sound, i);
        }
    }

    if(poured_from_)
    {
        pour_ += float(size) / (kPourSeconds * sample_rate_);
        if(pour_ >= 1.0f)
        {
            pour_        = 1.0f;
            poured_from_ = nullptr;
        }
    }

    // Each grain through the block: its state stays in registers
    std::fill(out[0], out[0] + size, 0.0f);
    std::fill(out[1], out[1] + size, 0.0f);
    Grain::Sums sums;
    for(auto& grain : grains_)
        grain.Render(grain_inc, out[0], out[1], size, sums);
    float power_sum = 0.0f;
    for(size_t i = 0; i < size; i++)
        power_sum += out[0][i] * out[0][i] + out[1][i] * out[1][i];

    const float n = float(size);
    grain_level_.store(sums.window / n, std::memory_order_relaxed);
    Normalise(sums.power / n, power_sum / n, sums.window_power / n, size);

    // ReverbSc, measured with a 10 kHz lowpass: T60 = 3 * kLoop /
    // -log10(feedback) within 20 %, output gain kGain / sqrt(1 - feedback^2)
    // within 1.3 dB from feedback 0.5 to 0.98. The inverse gain keeps the
    // reverb at the level of the grains.
    static constexpr float kLoop  = 0.06f; // seconds
    static constexpr float kGain  = 0.29f;
    const float            reverb = std::clamp(p.reverb, 0.0f, 1.0f);
    const float            decay
        = kReverbDecayMin * std::pow(kReverbDecayMax / kReverbDecayMin, reverb);
    const float feedback = std::pow(10.0f, -3.0f * kLoop / decay);
    reverb_.SetFeedback(feedback);
    const float reverb_gain     = std::sqrt(1.0f - feedback * feedback) / kGain;
    const float reverb_angle    = HALFPI_F * reverb * kReverbMaxAmount;
    const float reverb_dry_step = (std::cos(reverb_angle) - reverb_dry_) / n;
    const float reverb_wet_step
        = (reverb_gain * std::sin(reverb_angle) - reverb_wet_) / n;

    const float filter_coeff
        = 1.0f - std::exp(-n / (kFilterSeconds * sample_rate_));
    daisysp::fonepole(filter_, std::clamp(p.filter, -1.0f, 1.0f), filter_coeff);
    const float amount  = std::fabs(filter_);
    const bool  lowpass = filter_ < 0.0f;
    const float filter_hz
        = lowpass
              ? kLowpassOffHz * std::pow(kLowpassMinHz / kLowpassOffHz, amount)
              : kHighpassOffHz
                    * std::pow(kHighpassMaxHz / kHighpassOffHz, amount);
    const float filter_q   = kFilterQMin + amount * (kFilterQMax - kFilterQMin);
    const float filter_mix = std::min(1.0f, amount / kFilterFade);
    for(auto& filter : filters_)
        filter.Set(filter_hz, filter_q, sample_rate_);

    const float mute_target = p.mute ? 0.0f : 1.0f;
    const float mute_step   = 1.0f / (kMuteSeconds * sample_rate_);
    const float norm_step   = (norm_ - norm_gain_) / n;
    const float input_step  = (std::max(p.input, 0.0f) - input_gain_) / n;
    const float output_step
        = (std::clamp(p.output, 0.0f, 1.0f) - output_gain_) / n;
    // Dry and wet, equal power, exact at the ends
    const float mix       = std::clamp(p.mix, 0.0f, 1.0f);
    const float mix_angle = HALFPI_F * mix;
    const float dry       = mix >= 1.0f ? 0.0f : std::cos(mix_angle);
    const float wet       = mix <= 0.0f ? 0.0f : std::sin(mix_angle);
    const float dry_step  = (dry - dry_gain_) / n;
    const float wet_step  = (wet - wet_gain_) / n;
    for(size_t i = 0; i < size; i++)
    {
        reverb_dry_ += reverb_dry_step;
        reverb_wet_ += reverb_wet_step;
        norm_gain_ += norm_step;
        float l = out[0][i] * norm_gain_;
        float r = out[1][i] * norm_gain_;
        filters_[0].Process(l);
        filters_[1].Process(r);
        l += filter_mix
             * ((lowpass ? filters_[0].Low() : filters_[0].High()) - l);
        r += filter_mix
             * ((lowpass ? filters_[1].Low() : filters_[1].High()) - r);
        mute_gain_ = mute_gain_ < mute_target
                         ? std::min(mute_target, mute_gain_ + mute_step)
                         : std::max(mute_target, mute_gain_ - mute_step);
        l *= mute_gain_;
        r *= mute_gain_;
        float reverb_l, reverb_r;
        reverb_.Process(l, r, &reverb_l, &reverb_r);
        out[0][i] = l * reverb_dry_ + reverb_l * reverb_wet_;
        out[1][i] = r * reverb_dry_ + reverb_r * reverb_wet_;

        // The input, always into the ring
        input_gain_ += input_step;
        const float in_l = in[0][i] * input_gain_;
        const float in_r = in[1][i] * input_gain_;
        ring_.Write(dc_block_.Process(kInputSum * (in_l + in_r)));

        dry_gain_ += dry_step;
        wet_gain_ += wet_step;
        out[0][i] = out[0][i] * kOutputGain * wet_gain_ + in_l * dry_gain_;
        out[1][i] = out[1][i] * kOutputGain * wet_gain_ + in_r * dry_gain_;
        limiter_.Process(out[0][i], out[1][i]);
        output_gain_ += output_step;
        out[0][i] *= output_gain_;
        out[1][i] *= output_gain_;
    }

    UpdateCapture(size);
}

void Core::UpdateCapture(size_t size)
{
    const bool record = record_.load(std::memory_order_relaxed);
    if(record && !was_record_)
        held_ = 0;
    if(record)
        held_ += size;
    // Release: a new capture replaces one in progress
    if(!record && was_record_)
    {
        const size_t tap      = size_t(kTapSeconds * sample_rate_);
        const size_t pre_roll = size_t(kPreRollSeconds * sample_rate_);
        const bool   tapped   = held_ < tap;
        capture_back_         = tapped ? held_ : 0;
        capture_length_
            = tapped ? size_t(tap_seconds_.load(std::memory_order_relaxed)
                              * sample_rate_)
                     : held_ + pre_roll;
        capture_ = Capture::Waiting;
    }
    was_record_ = record;

    const size_t playing = playing_.load(std::memory_order_relaxed);
    Buffer&      target  = buffers_[1 - playing];
    if(capture_ == Capture::Waiting)
    {
        // The recording before is still pouring out, or grains started on it
        // still read it. Only with swaps closer than the pour and the longest
        // grain.
        if(poured_from_)
        {
            capture_back_ += size;
            return;
        }
        for(const auto& grain : grains_)
        {
            if(grain.IsActive() && grain.Source() == &target)
            {
                capture_back_ += size;
                return;
            }
        }
        // Waited long: the oldest part is lost to the writing on
        const size_t margin = size_t(kCopyMarginSeconds * sample_rate_);
        const size_t room   = ring_.Capacity() > capture_back_ + margin
                                ? ring_.Capacity() - capture_back_ - margin
                                : 0;
        ring_.Select(capture_back_, std::min(capture_length_, room));
        capture_ = Capture::Copying;
    }
    if(capture_ != Capture::Copying)
        return;
    const Ring::Step step = ring_.Copy(target, kCopyPerSample * size);
    if(step == Ring::Step::Pending)
        return;
    capture_ = Capture::Idle;
    if(step != Ring::Step::Done)
        return; // silence changes nothing
    // Pours in over the one before, if there is one
    poured_from_
        = buffers_[playing].Length() > 0 ? &buffers_[playing] : nullptr;
    pour_ = poured_from_ ? 0.0f : 1.0f;
    playing_.store(1 - playing, std::memory_order_release);
    restart_.store(true, std::memory_order_relaxed);
    recordings_.fetch_add(1, std::memory_order_release);
}

void Core::Normalise(float  power_grains,
                     float  power_sum,
                     float  window_power,
                     size_t size)
{
    const float coeff
        = 1.0f - std::exp(-float(size) / (kNormSeconds * sample_rate_));
    daisysp::fonepole(power_grains_, power_grains, coeff);
    daisysp::fonepole(power_sum_, power_sum, coeff);
    daisysp::fonepole(window_power_, window_power, coeff);

    // Uncorrelated grains each carry the input's power times their squared
    // window, split over two channels by the equal-power pan: sqrt(2 / window
    // power) gives each channel the input's power. At least 1: a lone grain
    // peaks at the input level.
    const float overlap_gain = std::sqrt(2.0f / std::max(window_power_, 1.0f));

    // Uncorrelated grains: the sum has the power of the grains, 1. Correlated
    // grains add in amplitude: up to 8 dB above that power at density 8, 11 dB
    // at 16. Never above 1: grains that cancel stay quiet. Silence: hold the
    // last value.
    if(power_sum_ >= 1e-10f)
        coherence_gain_ = std::min(1.0f, std::sqrt(power_grains_ / power_sum_));

    // 0.9 headroom
    norm_ = 0.9f * overlap_gain * coherence_gain_;
}

void Core::StartGrain(const Params& p,
                      const Source& source,
                      const Buffer* sound,
                      size_t        offset)
{
    Grain* grain  = nullptr;
    size_t active = 0;
    for(auto& g : grains_)
    {
        if(g.IsActive())
            active++;
        else if(!grain)
            grain = &g;
    }
    if(!grain)
        return;

    Grain::Latch latch;
    latch.buffer   = sound;
    double to_read = 1.0; // from the recording playing to the one read
    if(poured_from_ && 0.5f + 0.5f * Random() >= pour_)
    {
        latch.buffer = poured_from_;
        to_read      = double(poured_from_->Length()) / double(sound->Length());
    }
    // Each as Beads, over its range: position inside what is playing, size
    // between the knob's ends, exponential, any shape, pitch either way
    const double phase
        = p.rand_pos != 0.0f
              ? source.phase
                    + Blend(p.rand_pos) * (0.5 + 0.5 * Random() - source.phase)
              : source.phase;
    latch.position = (source.start + phase * source.length) * to_read;
    if(p.rand_size != 0.0f && p.max_grain_ms > p.min_grain_ms)
    {
        const float octaves = std::log2(p.max_grain_ms / p.min_grain_ms);
        const float size    = std::clamp(
            std::log2(p.grain_ms / p.min_grain_ms) / octaves, 0.0f, 1.0f);
        latch.size = std::exp2((Randomise(size, p.rand_size) - size) * octaves);
    }
    latch.shape = p.rand_shape != 0.0f ? Randomise(p.window_shape, p.rand_shape)
                                       : p.window_shape;
    const float semitones
        = p.rand_pitch != 0.0f
              ? Random() * Blend(p.rand_pitch) * kRandPitchSemitones
              : 0.0f;
    latch.rate = p.pitch * std::exp2(semitones / 12.0f);
    if(source.backwards)
        latch.rate = -latch.rate;
    // Width with the grains sounding and how much they differ: a lone grain
    // near the centre, most of the others too
    const float width = 0.5f * float(active + 1) * p.spread;
    const float scale = 0.5f - 0.5f * Random();
    latch.pan         = 0.5f + 0.5f * Random() * std::min(width * scale, 1.0f);
    grain->Start(latch, offset);

    // Keeps voices free for the next grains
    if(active + 1 + kSafetyVoices <= kNumVoices)
        return;
    Grain* furthest = nullptr;
    for(auto& g : grains_)
    {
        if(&g == grain || !g.IsActive() || g.IsFading())
            continue;
        if(!furthest || g.Progress() > furthest->Progress())
            furthest = &g;
    }
    if(furthest)
        furthest->FadeOut(size_t(kFadeSeconds * sample_rate_));
}

bool Core::Due(Stream& s, double sched_inc, float rand_density)
{
    s.sched_phase += sched_inc;
    if(s.sched_phase < s.sched_next)
        return false;
    s.sched_phase -= s.sched_next;
    s.sched_next = Interval(rand_density);
    // At most one start per sample
    s.sched_phase = std::min(s.sched_phase, double(s.sched_next));
    return true;
}

float Core::Interval(float rand_density)
{
    // Mean 1 at any amount. Up: an even spread. Down, at -1: Beads' random
    // starts, nearly Poisson without the closest starts and the longest gaps.
    // kMean: of the series on 0.1..1.
    if(rand_density >= 0.0f)
        return 1.0f + std::min(rand_density, 1.0f) * Random();
    static constexpr float kMean = 0.883f;
    const float            x     = 0.1f + 0.9f * (0.5f - 0.5f * Random());
    const float            random
        = x * (1.0f + x * (0.5f + x * (0.3333f + x * 0.25f))) / kMean;
    const float amount = std::min(-rand_density, 1.0f);
    return 1.0f - amount + amount * random;
}

float Core::Randomise(float x, float amount)
{
    return x + Blend(amount) * (0.5f + 0.5f * Random() - x);
}

float Core::Blend(float amount)
{
    return amount > 0.0f
               ? std::min(amount, 1.0f)
               : std::min(-3.0f * amount * (0.5f + 0.5f * Random()), 1.0f);
}

float Core::Random()
{
    // xorshift32
    seed_ ^= seed_ << 13;
    seed_ ^= seed_ >> 17;
    seed_ ^= seed_ << 5;
    return float(int32_t(seed_)) * (1.0f / 2147483648.0f);
}

} // namespace clessidra
