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
    const Params p         = slots_[slot_.load(std::memory_order_acquire)];
    Buffer&      sound     = buffers_[playing_.load(std::memory_order_relaxed)];
    const double len       = double(sound.Length());
    const double sel_len   = std::max(double(p.length) * len, 1.0);
    const double sel_start = double(p.start) * len;
    const float  density   = std::clamp(p.density, 0.0f, kMaxDensity);
    // A grain lasts grain_samples whatever its pitch, as the starts per second
    // do, no longer than the recording
    const double grain_samples = std::clamp(
        p.grain_ms * 0.001 * sample_rate_, 1.0, std::max(len, 1.0));
    const double grain_inc = 1.0 / grain_samples;
    // Nothing recorded: no grains, the size as set
    grain_ms_
        = len > 0.0 ? float(1000.0 * grain_samples / sample_rate_) : p.grain_ms;

    // A new length keeps the playhead where it is in the recording. Past the
    // new end: what End does there.
    if(sel_len_ > 0.0 && sel_len != sel_len_)
        playhead_.Scale(sel_len_ / sel_len);
    sel_len_ = sel_len;

    if(restart_)
    {
        restart_ = false;
        playhead_.Restart(p.speed < 0.0f);
        // With a grain at once
        schedule_.phase = schedule_.next;
    }
    if(turn_.exchange(false, std::memory_order_relaxed))
        playhead_.Turn();
    // A grain at once, the starts counted from it
    const float jump = jump_.exchange(-1.0f, std::memory_order_relaxed);
    if(jump >= 0.0f)
    {
        playhead_.Jump(jump);
        schedule_.phase = 0.0;
    }
    const bool now = jump >= 0.0f && len > 0.0;

    const double play_inc = p.freeze ? 0.0 : p.speed / sel_len; // per sample

    const size_t block = std::min(size, kMaxBlock);

    // Grain starts in this block, at the sample they fall on. A voice that ends
    // in this block is free from the next.
    for(size_t i = 0; i < size && len > 0.0; i++)
    {
        // Through the selection, as End sets at its edges. Run out: no grains
        // but one at once.
        playhead_.Advance(play_inc, p.end);
        const double phase = playhead_.Phase();
        const bool   due
            = !playhead_.Ended()
              && Due(schedule_, density / sample_rate_, p.random_starts);
        if(due || (now && i == 0))
        {
            const bool reverse = (p.speed < 0.0f) != (playhead_.Heading() < 0);
            StartGrain(p, {sel_start, sel_len, phase, reverse}, &sound, i);
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

    // Each grain through the block, into the mix and, by its send, the
    // reverb: its state stays in registers
    float send_l[kMaxBlock], send_r[kMaxBlock];
    std::fill(out[0], out[0] + size, 0.0f);
    std::fill(out[1], out[1] + size, 0.0f);
    std::fill(send_l, send_l + block, 0.0f);
    std::fill(send_r, send_r + block, 0.0f);
    Grain::Sums sums;
    for(auto& grain : grains_)
        grain.Render(grain_inc, out[0], out[1], send_l, send_r, block, sums);
    float power_sum = 0.0f;
    for(size_t i = 0; i < block; i++)
        power_sum += out[0][i] * out[0][i] + out[1][i] * out[1][i];

    const float n = float(block);
    grain_level_.store(sums.window / n, std::memory_order_relaxed);
    Normalise(sums.power / n, power_sum / n, sums.window_power / n, block);

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
    for(size_t i = 0; i < block; i++)
    {
        reverb_dry_ += reverb_dry_step;
        reverb_wet_ += reverb_wet_step;
        norm_gain_ += norm_step;
        float l    = out[0][i] * norm_gain_;
        float r    = out[1][i] * norm_gain_;
        mute_gain_ = mute_gain_ < mute_target
                         ? std::min(mute_target, mute_gain_ + mute_step)
                         : std::max(mute_target, mute_gain_ - mute_step);
        l *= mute_gain_;
        r *= mute_gain_;
        const float gain = norm_gain_ * mute_gain_;
        float       reverb_l, reverb_r;
        reverb_.Process(
            send_l[i] * gain, send_r[i] * gain, &reverb_l, &reverb_r);

        // The input, always into the ring
        input_gain_ += input_step;
        const float in_l = in[0][i] * input_gain_;
        const float in_r = in[1][i] * input_gain_;
        const float mono = dc_block_.Process(kInputSum * (in_l + in_r));
        ring_.Write(mono);

        out[0][i] = l * reverb_dry_ + reverb_l * reverb_wet_;
        out[1][i] = r * reverb_dry_ + reverb_r * reverb_wet_;
        dry_gain_ += dry_step;
        wet_gain_ += wet_step;
        out[0][i] = out[0][i] * kOutputGain * wet_gain_ + in_l * dry_gain_;
        out[1][i] = out[1][i] * kOutputGain * wet_gain_ + in_r * dry_gain_;
        limiter_.Process(out[0][i], out[1][i]);
        output_gain_ += output_step;
        out[0][i] *= output_gain_;
        out[1][i] *= output_gain_;
    }
    for(size_t i = block; i < size; i++)
        out[0][i] = out[1][i] = 0.0f;
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
    const size_t next    = 1 - playing;
    Buffer&      target  = buffers_[next];
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
    playing_.store(next, std::memory_order_release);
    restart_ = true;
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
    {
        dropped_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    Grain::Latch latch;
    latch.buffer   = sound;
    double to_read = 1.0; // from the recording playing to the one read
    if(poured_from_ && 0.5f + 0.5f * Random() >= pour_)
    {
        latch.buffer = poured_from_;
        to_read      = double(poured_from_->Length()) / double(sound->Length());
    }
    // Each evenly over its range: position within the amount around the
    // playhead, round the selection, size between its ends, exponential,
    // pitch either way, level, reverb send
    double phase = source.phase;
    if(p.rand_pos > 0.0f)
    {
        phase += 0.5 * std::min(p.rand_pos, 1.0f) * Random();
        phase -= std::floor(phase);
    }
    latch.position = (source.start + phase * source.length) * to_read;
    if(p.rand_size > 0.0f && p.max_grain_ms > p.min_grain_ms)
    {
        const float octaves = std::log2(p.max_grain_ms / p.min_grain_ms);
        const float size    = std::clamp(
            std::log2(p.grain_ms / p.min_grain_ms) / octaves, 0.0f, 1.0f);
        latch.size = std::exp2((Randomise(size, p.rand_size) - size) * octaves);
    }
    latch.shape = p.window_shape;
    const float semitones
        = Random() * std::clamp(p.rand_pitch, 0.0f, 1.0f) * kRandPitchSemitones;
    latch.rate = p.pitch * std::exp2(semitones / 12.0f);
    if(source.backwards)
        latch.rate = -latch.rate;
    if(p.rand_gain > 0.0f)
        latch.gain = std::pow(10.0f,
                              kRandGainDb / 20.0f * std::min(p.rand_gain, 1.0f)
                                  * (0.5f + 0.5f * Random()));
    latch.send = Randomise(1.0f, p.rand_send);
    // Width with the grains sounding and how much they differ: a lone grain
    // near the centre, most of the others too
    const float width = 0.5f * float(active + 1) * p.spread;
    const float scale = 0.5f - 0.5f * Random();
    latch.pan         = 0.5f + 0.5f * Random() * std::min(width * scale, 1.0f);
    grain->Start(latch, offset);
}

bool Core::Due(Schedule& s, double sched_inc, bool random)
{
    // None at no density, not even the grain at once of a restart
    if(sched_inc <= 0.0)
        return false;
    s.phase += sched_inc;
    if(s.phase < s.next)
        return false;
    s.phase -= s.next;
    s.next = Interval(random);
    // At most one start per sample
    s.phase = std::min(s.phase, double(s.next));
    return true;
}

float Core::Interval(bool random)
{
    if(!random)
        return 1.0f;
    // -ln(1 - x) to its fourth term, x even on 0.1..1: nearly exponential
    // (Poisson) without the closest starts and the longest gaps. kMean: of the
    // series, for a mean of 1.
    static constexpr float kMean = 0.883f;
    const float            x     = 0.1f + 0.9f * (0.5f - 0.5f * Random());
    return x * (1.0f + x * (0.5f + x * (0.3333f + x * 0.25f))) / kMean;
}

float Core::Randomise(float x, float amount)
{
    return x + std::clamp(amount, 0.0f, 1.0f) * (0.5f + 0.5f * Random() - x);
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
