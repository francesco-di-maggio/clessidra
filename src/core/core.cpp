#include "core.h"

#include <algorithm>
#include <cmath>
#include <Utility/dsp.h>

namespace clessidra
{
void Core::Init(float         sample_rate,
                float*        ring,
                size_t        ring_size,
                float* const* buffers,
                size_t        buffer_size)
{
    sample_rate_ = sample_rate;
    ring_.Init(ring, ring_size, sample_rate);
    ring_size_ = ring_size;
    for(size_t b = 0; b < kNumBuffers; b++)
        buffers_[b].Init(buffers[b], buffer_size, sample_rate);
    dc_block_.Init(sample_rate);
    reverb_.Init(sample_rate);
    reverb_.SetLpFreq(kReverbLowpassHz);
    limiter_.Init(sample_rate);

    Grain::InitWindow();
}

void Core::Process(const float* const* in, float** out, size_t size)
{
    const Params p         = params_;
    const double len       = double(buffers_[playing_].Length());
    const double sel_start = p.start * len;
    const double sel_len   = std::max(p.length * len, 1.0);
    const double place_len = std::max(sel_len / kNumPlaces, 1.0);
    // A grain reads about grain_samples: no longer than the recording
    const double grain_samples = std::clamp(
        p.grain_ms * 0.001 * sample_rate_, 1.0, std::max(len, 1.0));
    const double grain_inc = p.pitch / grain_samples;
    // Sizes log-uniform within +-octaves: their mean, which the starts divide
    // by so the density stays the grains at once
    const float octaves
        = std::clamp(p.size_variation, 0.0f, 1.0f) * kSizeVariationOctaves;
    const float mean_size = octaves > 0.0f
                                ? (std::exp2(octaves) - std::exp2(-octaves))
                                      / (2.0f * octaves * float(M_LN2))
                                : 1.0f;

    // Freeze holds every source where it is; position moves them from there
    if(p.freeze && !was_freeze_)
    {
        for(auto& stream : streams_)
            stream.frozen = stream.phase;
        freeze_position_ = p.position;
    }
    was_freeze_        = p.freeze;
    const double scrub = double(p.position) - double(freeze_position_);

    struct Span
    {
        double start, length;
        double play_inc;  // phase per sample
        double sched_inc; // density starts per grain duration
    };
    std::array<Span, kNumStreams> spans;
    for(size_t s = 0; s < kNumStreams; s++)
    {
        const bool  playhead = s == 0;
        const float density  = std::clamp(
            playhead ? p.density : p.place_density[s - 1], 0.0f, kMaxDensity);
        const double length = playhead ? sel_len : place_len;
        spans[s]            = {playhead ? sel_start
                             : sel_start + double(s - 1) * sel_len / kNumPlaces,
                    length,
                    p.speed / length,
                    grain_inc * density / mean_size};

        Stream&    stream = streams_[s];
        const bool on     = density > 0.0f;
        // A place starts at its edge in the playing direction, with a grain at
        // once. The playhead keeps running.
        if(on && !stream.on && !playhead)
        {
            stream.phase  = p.speed < 0.0f ? std::nextafter(1.0, 0.0) : 0.0;
            stream.frozen = stream.phase - scrub;
            stream.sched_phase = stream.sched_next;
        }
        stream.on = on || playhead;
        if(p.freeze)
        {
            stream.phase = stream.frozen + scrub;
            stream.phase -= std::floor(stream.phase);
        }
    }

    // Grain starts in this block, at the sample they fall on. A voice that ends
    // in this block is free from the next.
    for(size_t i = 0; i < size; i++)
    {
        for(size_t s = 0; s < kNumStreams; s++)
        {
            Stream&     stream = streams_[s];
            const Span& span   = spans[s];
            if(!stream.on)
                continue;
            if(!p.freeze)
            {
                stream.phase += span.play_inc;
                stream.phase -= std::floor(stream.phase);
            }

            stream.sched_phase += span.sched_inc;
            if(stream.sched_phase >= stream.sched_next)
            {
                stream.sched_phase -= stream.sched_next;
                Trigger(p,
                        span.start,
                        span.length,
                        stream.phase,
                        s == 0 ? p.level : p.place_level[s - 1],
                        i);
                stream.sched_next = Interval(p.jitter);
                // At most one start per sample
                stream.sched_phase
                    = std::min(stream.sched_phase, double(stream.sched_next));
            }
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
    grain_level_  = sums.window / n;
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

    // Dry/wet and what the ring records, equal power. The wet signal is
    // recorded whatever the dry/wet.
    const float angle           = HALFPI_F * std::clamp(p.mix, 0.0f, 1.0f);
    const float record_angle    = HALFPI_F * std::clamp(p.resample, 0.0f, 1.0f);
    const float norm_step       = (norm_ - norm_gain_) / n;
    const float wet_step        = (std::sin(angle) - wet_gain_) / n;
    const float dry_step        = (std::cos(angle) - dry_gain_) / n;
    const float record_wet_step = (std::sin(record_angle) - record_wet_) / n;
    const float record_input_step
        = (std::cos(record_angle) - record_input_) / n;
    const float input_step = (std::max(p.input, 0.0f) - input_gain_) / n;
    const float output_step
        = (std::clamp(p.output, 0.0f, 1.0f) - output_gain_) / n;
    for(size_t i = 0; i < size; i++)
    {
        reverb_dry_ += reverb_dry_step;
        reverb_wet_ += reverb_wet_step;
        norm_gain_ += norm_step;
        wet_gain_ += wet_step;
        dry_gain_ += dry_step;
        record_wet_ += record_wet_step;
        record_input_ += record_input_step;
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
        l = l * reverb_dry_ + reverb_l * reverb_wet_;
        r = r * reverb_dry_ + reverb_r * reverb_wet_;

        // Input as before; the wet signal in mono at the level of the input,
        // undoing the equal-power pan
        input_gain_ += input_step;
        const float in_l  = in[0][i] * input_gain_;
        const float in_r  = in[1][i] * input_gain_;
        const float input = dc_block_.Process(0.5f * (in_l + in_r));
        ring_.Write(record_input_ * input
                    + record_wet_ * float(M_SQRT1_2) * (l + r));

        out[0][i] = l * wet_gain_ + in_l * dry_gain_;
        out[1][i] = r * wet_gain_ + in_r * dry_gain_;
        limiter_.Process(out[0][i], out[1][i]);
        output_gain_ += output_step;
        out[0][i] *= output_gain_;
        out[1][i] *= output_gain_;
    }

    UpdateCapture(size);
}

void Core::UpdateCapture(size_t size)
{
    const bool record = record_;
    if(record && !was_record_)
        held_ = 0;
    if(record)
        held_ += size;
    // Release: a new capture replaces one in progress
    if(!record && was_record_)
    {
        const size_t tap      = size_t(kTapSeconds * sample_rate_);
        const size_t pre_roll = size_t(kPreRollSeconds * sample_rate_);
        capture_back_         = held_ < tap ? held_ : 0;
        capture_length_
            = held_ < tap ? buffers_[0].Capacity() : held_ + pre_roll;
        capture_ = Capture::Waiting;
    }
    was_record_ = record;

    Buffer& target = buffers_[1 - playing_];
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
        const size_t room   = ring_size_ > capture_back_ + margin
                                ? ring_size_ - capture_back_ - margin
                                : 0;
        ring_.Select(capture_back_,
                     std::min({capture_length_, room, target.Capacity()}));
        capture_ = Capture::Copying;
    }
    if(capture_ == Capture::Copying)
    {
        const Ring::Step step = ring_.Copy(target, kCopyPerBlock);
        if(step == Ring::Step::Pending)
            return;
        capture_ = Capture::Idle;
        if(step == Ring::Step::Done)
            playing_ = 1 - playing_;
    }
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
                   double        start,
                   double        length,
                   double        phase,
                   float         level,
                   size_t        offset)
{
    const Buffer& buffer = buffers_[playing_];
    if(buffer.Length() == 0)
        return; // nothing recorded: no grains

    Grain* grain = nullptr;
    for(auto& g : grains_)
    {
        if(!g.IsActive())
        {
            grain = &g;
            break;
        }
    }
    if(!grain)
        return;

    Grain::Latch latch;
    latch.buffer = &buffer;
    latch.position
        = start + phase * length + Random() * p.rand_pos * length * 0.5;
    float ratio = std::exp2(Random() * p.rand_pitch / 12.0f);
    // Snapped: the nearest partial of the fundamental two octaves below, 1/4 ..
    // 4 within the +-24 semitones of random pitch
    if(p.harmonic > 0.0f && 0.5f - 0.5f * Random() <= p.harmonic)
    {
        ratio = std::max(1.0f, std::round(4.0f * ratio)) / 4.0f;
    }
    latch.pan  = 0.5f + Random() * p.spread * 0.5f;
    latch.rate = p.pitch * ratio;
    // Grains read in the playhead's direction, a share of them the other way
    bool backwards = !p.freeze && p.speed < 0.0f;
    if(p.reverse > 0.0f && 0.5f - 0.5f * Random() <= p.reverse)
        backwards = !backwards;
    if(backwards)
        latch.rate = -latch.rate;
    latch.gain = level;
    if(p.size_variation > 0.0f)
        latch.size
            = std::exp2(Random() * p.size_variation * kSizeVariationOctaves);
    latch.shape = p.window_shape;
    grain->Start(latch, offset);
}

float Core::Interval(float jitter)
{
    // Mean 1 at any jitter. Exponential intervals (jitter 1) make Poisson
    // starts.
    const float u           = 0.5f - 0.5f * Random(); // (0, 1]
    const float exponential = -std::log(u);
    return 1.0f - jitter + jitter * exponential;
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
