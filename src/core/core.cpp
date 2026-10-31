#include "core.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <Utility/dsp.h>

namespace clessidra
{
void Core::Init(float sample_rate, float* buffer, size_t buffer_size)
{
    sample_rate_ = sample_rate;
    buffer_.Init(buffer, buffer_size);
    write_ = std::min(size_t(kStartLagSeconds * sample_rate), buffer_size - 1);
    dc_block_.Init(sample_rate);
    reverb_.Init(sample_rate);
    reverb_.SetLpFreq(kReverbLowpassHz);
    limiter_.Init(sample_rate);

    Grain::InitWindow();
}

void Core::Process(const float* const* in, float** out, size_t size)
{
    const Params p       = params_;
    const bool   writing = p.fill > 0.0f || fill_gain_ > 0.0f;
    const size_t sound   = wrapped_ ? buffer_.Length() : write_;
    const double len     = double(writing ? buffer_.Length() : sound);
    const double lag     = double(size_t(kStartLagSeconds * sample_rate_));
    // Only what comes in: the held sound ends at it, for when Fill comes down
    size_t held_end = held_end_.load(std::memory_order_relaxed);
    if(p.fill >= 1.0f)
        held_end = write_;
    // The held sound stays where it is. The grains that follow the input read
    // just behind the write point.
    const double sel_len = std::max(p.length * double(sound), 1.0);
    const double sel_start
        = double(held_end) - p.start * double(sound) - sel_len;
    const double live_len   = std::clamp(p.length * double(buffer_.Length()),
                                       1.0,
                                       std::max(double(sound) - lag, 1.0));
    const double live_start = double(write_) - lag - live_len;
    const float  density    = std::clamp(p.density, 0.0f, kMaxDensity);
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
    const double low       = p.slice_start;
    const double high      = p.slice_end;
    const double live_low  = p.live_slice_start;
    const double live_high = p.live_slice_end;
    const double live_top  = std::nextafter(live_high, live_low);
    // Only what comes in: the held sound starts over when Fill comes down
    const bool restart = restart_.exchange(false, std::memory_order_relaxed);
    if(restart || p.fill >= 1.0f)
    {
        playhead_.Restart();
        stream_.phase = p.speed < 0.0f ? std::nextafter(high, low) : low;
    }
    if(restart)
    {
        stream_.live_phase = p.speed < 0.0f ? live_top : live_low;
        // With a grain at once
        stream_.sched_phase = stream_.sched_next;
    }
    // Opening the glass: what comes in, at once
    if(writing && !was_writing_)
    {
        filled_             = true;
        stream_.live_phase  = live_top;
        stream_.sched_phase = stream_.sched_next;
    }
    was_writing_ = writing;
    // At speed 1, what comes in now
    if(writing && !p.freeze && p.speed == 1.0f)
        stream_.live_phase = live_top;

    // Outside Loop, the last grains stay inside the slice
    const double margin = std::min(grain_samples * std::fabs(p.pitch) / sel_len,
                                   0.25 * (high - low));

    const double play_inc
        = p.freeze ? 0.0 : p.speed / sel_len; // phase per sample
    const double live_inc   = p.freeze ? 0.0 : p.speed / live_len;
    const double live_drift = 1.0 / live_len;
    // grain starts per sample
    const double sched_inc = density / sample_rate_;

    // Grain starts in this block, at the sample they fall on. A voice that ends
    // in this block is free from the next.
    for(size_t i = 0; i < size; i++)
    {
        if(!p.freeze)
            stream_.phase = playhead_.Step(
                stream_.phase, play_inc, p.end, low, high, margin);
        // What comes in has no end to reach: it wraps
        if(writing)
            stream_.live_phase = live_playhead_.Step(stream_.live_phase,
                                                     live_inc,
                                                     Playhead::End::Loop,
                                                     live_low,
                                                     live_high,
                                                     0.0,
                                                     live_drift);
        if(playhead_.IsStopped() && !writing)
            continue;

        stream_.sched_phase += sched_inc;
        if(stream_.sched_phase >= stream_.sched_next)
        {
            stream_.sched_phase -= stream_.sched_next;
            // Each grain falls from what comes in or from what is held: Fill
            // sets the odds
            if(0.5f + 0.5f * Random() < p.fill)
            {
                const Source live = {live_start + double(i),
                                     live_len,
                                     stream_.live_phase,
                                     live_low,
                                     live_high,
                                     live_inc < 0.0,
                                     double(write_) - lag + double(i),
                                     0.0};
                Trigger(p, live, i, grain_samples);
            }
            else if(!playhead_.IsStopped())
            {
                const Source held
                    = {sel_start,
                       sel_len,
                       stream_.phase,
                       low,
                       high,
                       !p.freeze && playhead_.IsBackward(play_inc),
                       std::numeric_limits<double>::max(),
                       wrapped_ ? 0.0 : double(sound)};
                Trigger(p, held, i, grain_samples);
            }
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
        grain.Render(
            grain_inc, out[0], out[1], size, sums, double(write_), writing);
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
    const float fill_step  = (std::clamp(p.fill, 0.0f, 1.0f) - fill_gain_) / n;
    const float input_step = (std::max(p.input, 0.0f) - input_gain_) / n;
    const float output_step
        = (std::clamp(p.output, 0.0f, 1.0f) - output_gain_) / n;
    float* const memory = buffer_.Data();
    const size_t length = buffer_.Length();
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

        // The input over what is there. A blend: it can't build up.
        input_gain_ += input_step;
        fill_gain_ += fill_step;
        const float input = dc_block_.Process(kInputSum * (in[0][i] + in[1][i])
                                              * input_gain_);
        if(writing)
        {
            memory[write_] += fill_gain_ * (input - memory[write_]);
            if(++write_ == length)
            {
                write_   = 0;
                wrapped_ = true;
            }
        }

        out[0][i] *= kOutputGain;
        out[1][i] *= kOutputGain;
        limiter_.Process(out[0][i], out[1][i]);
        output_gain_ += output_step;
        out[0][i] *= output_gain_;
        out[1][i] *= output_gain_;
    }

    held_end_.store(p.fill >= 1.0f ? write_ : held_end,
                    std::memory_order_relaxed);
    ended_.store(playhead_.IsStopped(), std::memory_order_relaxed);
    sound_samples_.store(wrapped_ ? length : write_, std::memory_order_relaxed);
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
                   size_t        offset,
                   double        grain_samples)
{
    if(!filled_)
        return; // nothing in the buffer: no grains

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
    latch.buffer = &buffer_;
    // Scatter stays inside what is playing, mirrored at its ends: wrapped, the
    // newest sound would read the oldest
    const double low       = source.low;
    const double high      = source.high;
    double       scattered = source.phase + Random() * p.rand_pos * 0.5;
    if(scattered > high)
        scattered = 2.0 * high - scattered;
    if(scattered < low)
        scattered = std::min(2.0 * low - scattered, high);
    latch.position = source.start + scattered * source.length;
    if(source.wrap > 0.0)
        latch.position
            -= source.wrap * std::floor(latch.position / source.wrap);
    // Peaky: the further each grain may go is itself random
    const float reach
        = std::min(3.0f * p.rand_pitch * (0.5f - 0.5f * Random()), 1.0f);
    const float ratio
        = std::exp2(Random() * reach * kRandPitchSemitones / 12.0f);
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
    // Reading faster than sound comes in: starts far enough back not to reach
    // what is being written, where the oldest sound is
    if(latch.rate > 1.0f)
        latch.position = std::min(latch.position,
                                  source.newest
                                      - grain_samples * double(latch.size)
                                            * (double(latch.rate) - 1.0));
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
