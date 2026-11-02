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

    if(p.freeze)
        playhead_.Resume();
    const double low  = p.slice_start;
    const double high = p.slice_end;
    if(restart_.exchange(false, std::memory_order_relaxed))
    {
        playhead_.Restart();
        stream_.phase = p.speed < 0.0f ? std::nextafter(high, low) : low;
        // With a grain at once
        stream_.sched_phase = stream_.sched_next;
    }

    // Outside Loop, the last grains stay inside the slice
    const double margin = std::min(grain_samples * std::fabs(p.pitch) / sel_len,
                                   0.25 * (high - low));
    const double play_inc
        = p.freeze ? 0.0 : p.speed / sel_len; // phase per sample
    // grain starts per sample
    const double sched_inc = density / sample_rate_;

    // Grain starts in this block, at the sample they fall on. A voice that ends
    // in this block is free from the next.
    for(size_t i = 0; i < size && len > 0.0; i++)
    {
        if(!p.freeze)
            stream_.phase = playhead_.Step(
                stream_.phase, play_inc, p.end, low, high, margin);
        if(playhead_.IsStopped())
            continue;

        stream_.sched_phase += sched_inc;
        if(stream_.sched_phase >= stream_.sched_next)
        {
            stream_.sched_phase -= stream_.sched_next;
            const Source source = {sel_start,
                                   sel_len,
                                   stream_.phase,
                                   low,
                                   high,
                                   !p.freeze && playhead_.IsBackward(play_inc)};
            Trigger(p, source, &sound, i);
            stream_.sched_next = Interval(p.jitter);
            // At most one start per sample
            stream_.sched_phase
                = std::min(stream_.sched_phase, double(stream_.sched_next));
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

    const float tone_coeff
        = 1.0f - std::exp(-n / (kToneSeconds * sample_rate_));
    daisysp::fonepole(tone_, std::clamp(p.tone, -1.0f, 1.0f), tone_coeff);
    const float tone    = std::fabs(tone_);
    const bool  lowpass = tone_ < 0.0f;
    const float tone_hz
        = lowpass
              ? kLowpassOffHz * std::pow(kLowpassMinHz / kLowpassOffHz, tone)
              : kHighpassOffHz
                    * std::pow(kHighpassMaxHz / kHighpassOffHz, tone);
    const float tone_q   = kToneQMin + tone * (kToneQMax - kToneQMin);
    const float tone_mix = std::min(1.0f, tone / kToneFade);
    for(auto& filter : tone_filter_)
        filter.Set(tone_hz, tone_q, sample_rate_);

    const float norm_step  = (norm_ - norm_gain_) / n;
    const float input_step = (std::max(p.input, 0.0f) - input_gain_) / n;
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
        tone_filter_[0].Process(l);
        tone_filter_[1].Process(r);
        l += tone_mix
             * ((lowpass ? tone_filter_[0].Low() : tone_filter_[0].High()) - l);
        r += tone_mix
             * ((lowpass ? tone_filter_[1].Low() : tone_filter_[1].High()) - r);
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

    ended_.store(playhead_.IsStopped(), std::memory_order_relaxed);
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
        capture_length_ = tapped ? size_t(kTapCaptureSeconds * sample_rate_)
                                 : held_ + pre_roll;
        capture_ = Capture::Waiting;
    }
    was_record_ = record;

    const size_t playing = playing_.load(std::memory_order_relaxed);
    Buffer&      target  = buffers_[1 - playing];
    if(capture_ == Capture::Waiting)
    {
        // Grains started before the last swap can still read it. Only with
        // swaps closer than the longest grain.
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

void Core::Trigger(const Params& p,
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
    latch.buffer = sound;
    // Scatter stays inside what is playing, mirrored at its ends
    const double low       = source.low;
    const double high      = source.high;
    double       scattered = source.phase + Random() * p.rand_pos * 0.5;
    if(scattered > high)
        scattered = 2.0 * high - scattered;
    if(scattered < low)
        scattered = std::min(2.0 * low - scattered, high);
    latch.position = source.start + scattered * source.length;
    // Peaky: the further each grain may go is itself random
    const float reach
        = std::min(3.0f * p.rand_pitch * (0.5f - 0.5f * Random()), 1.0f);
    const float semitones
        = Step(Random() * reach * kRandPitchSemitones, p.pitch_steps);
    const float ratio = std::exp2(semitones / 12.0f);
    // Width with the grains sounding and how much they differ: a lone grain
    // near the centre, most of the others too
    const float width = 0.5f * float(active + 1) * p.spread;
    const float scale = 0.5f - 0.5f * Random();
    latch.pan         = 0.5f + 0.5f * Random() * std::min(width * scale, 1.0f);
    latch.rate        = p.pitch * ratio;
    if(source.backwards)
        latch.rate = -latch.rate;
    if(p.size_variation > 0.0f)
        latch.size
            = std::exp2(Random() * p.size_variation * kSizeVariationOctaves);
    latch.shape = p.window_shape;
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

float Core::Step(float semitones, Steps steps)
{
    if(steps == Steps::Octaves)
        return 12.0f * std::round(semitones / 12.0f);
    if(steps == Steps::Free)
        return semitones;
    // The nearest of 0, 7, 12, 19 and 24, either way
    static constexpr float kFifths[] = {0.0f, 7.0f, 12.0f, 19.0f, 24.0f};
    const float            size      = std::fabs(semitones);
    float                  nearest   = 0.0f;
    for(float f : kFifths)
        if(std::fabs(size - f) < std::fabs(size - nearest))
            nearest = f;
    return std::copysign(nearest, semitones);
}

float Core::Interval(float jitter)
{
    // Mean 1 at any jitter. Random (jitter 1): nearly Poisson, without the
    // closest starts and the longest gaps. kMean: of the series on 0.1..1.
    static constexpr float kMean = 0.883f;
    const float            x     = 0.1f + 0.9f * (0.5f - 0.5f * Random());
    const float            random
        = x * (1.0f + x * (0.5f + x * (0.3333f + x * 0.25f))) / kMean;
    return 1.0f - jitter + jitter * random;
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
