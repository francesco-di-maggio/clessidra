#include "core.h"

#include "aligner.h"

#include <algorithm>
#include <cmath>
#include <cstdlib> // RAND_MAX, for DaisySP dsp.h
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
}

void Core::Process(const float* const* in, float** out, size_t size)
{
    const Params p     = slots_[slot_.load(std::memory_order_acquire)];
    Buffer&      sound = buffers_[playing_.load(std::memory_order_relaxed)];
    const double len   = double(sound.Length());
    // The loop ends no later than the sound's end
    const double length    = std::min(double(p.length), 1.0 - double(p.start));
    const double sel_len   = std::max(length * len, 1.0);
    const double sel_start = double(p.start) * len;
    const float  density   = std::clamp(p.density, 0.0f, kMaxDensity);
    // A grain lasts grain_samples whatever its pitch, as the starts per second
    // do, no longer than the recording
    const double grain_samples = std::clamp(
        p.grain_ms * 0.001 * sample_rate_, 1.0, std::max(len, 1.0));
    // Nothing recorded: no grains, the size as set
    grain_ms_
        = len > 0.0 ? float(1000.0 * grain_samples / sample_rate_) : p.grain_ms;

    if(restart_)
    {
        restart_ = false;
        playhead_.Restart(p.speed < 0.0f);
        // With a grain at once, unless rising from silence
        schedule_.phase = poured_from_ ? schedule_.next : 0.0;
    }
    // A grain at once, the starts counted from it
    const float jump = jump_.exchange(-1.0f, std::memory_order_relaxed);
    if(jump >= 0.0f)
    {
        playhead_.Jump(jump);
        schedule_.phase = 0.0;
        if(!p.stop)
            tape_ = 1.0f;
    }
    const bool  now   = jump >= 0.0f && len > 0.0;
    const float scrub = scrub_.exchange(0.0f, std::memory_order_relaxed);
    if(p.freeze && scrub != 0.0f)
    {
        const double to = playhead_.Phase() + double(scrub);
        playhead_.Jump(to - std::floor(to));
    }

    // Stop: the speed of everything ramps to 0 or back to 1 over
    // kStopSeconds, at a steady rate as a tape machine's
    const float tape_step = float(size) / (kStopSeconds * sample_rate_);
    tape_                 = p.stop ? std::max(0.0f, tape_ - tape_step)
                   : std::min(1.0f, tape_ + tape_step);
    const double play_inc
        = p.freeze ? 0.0 : p.speed * double(tape_) / sel_len; // per sample

    aligns_ = std::min(kAlignBurst,
                       aligns_ + kAlignsPerSecond * float(size) / sample_rate_);

    // The first recording rises from silence: its grains per second from 0
    const float rise = poured_from_ ? 1.0f : pour_;

    // Slices: each newly held one from its start, a grain at once. At no
    // density, two grains at once, regular: the slice as it is.
    const bool   slicing      = p.slices_held != 0;
    const float  slice_random = density > 0.0f ? p.rand_time : 0.0f;
    const double slice_density
        = density > 0.0f ? double(density) : 2.0 * sample_rate_ / grain_samples;
    for(size_t s = 0; s < kNumSlices; s++)
    {
        const uint32_t bit = 1u << s;
        if((p.slices_held & bit) && !(slices_held_ & bit))
        {
            slice_heads_[s].Restart(p.speed < 0.0f);
            slice_schedules_[s].phase = slice_schedules_[s].next;
        }
    }
    slices_held_ = p.slices_held;

    // Grain starts in this block, at the sample they fall on. A voice that ends
    // in this block is free from the next.
    for(size_t i = 0; i < size && len > 0.0; i++)
    {
        // Through the selection, as End sets at its edges. Run out: no grains
        // but one at once. Waiting while slices play.
        if(!slicing)
        {
            playhead_.Advance(play_inc, p.end);
            const bool due = !playhead_.Ended()
                             && Due(schedule_,
                                    density * tape_ * rise / sample_rate_,
                                    p.rand_time);
            if(due || (now && i == 0))
                StartGrain(p,
                           {sel_start,
                            sel_len,
                            playhead_.Phase(),
                            (p.speed < 0.0f) != (playhead_.Heading() < 0),
                            grain_samples},
                           &sound,
                           i,
                           last_);
        }
        for(size_t s = 0; s < kNumSlices && slicing; s++)
        {
            if(!(p.slices_held & (1u << s)))
                continue;
            const double span = std::max(p.slice_length[s] * sel_len, 1.0);
            Playhead&    head = slice_heads_[s];
            head.Advance(p.freeze ? 0.0 : p.speed * double(tape_) / span,
                         p.end);
            if(!head.Ended()
               && Due(slice_schedules_[s],
                      slice_density * tape_ / sample_rate_,
                      slice_random))
                StartGrain(p,
                           {sel_start + p.slice_start[s] * sel_len,
                            span,
                            head.Phase(),
                            (p.speed < 0.0f) != (head.Heading() < 0),
                            grain_samples},
                           &sound,
                           i,
                           slice_last_[s]);
        }
    }

    if(pour_ < 1.0f)
    {
        pour_ += float(size) / (kPourSeconds * sample_rate_);
        if(pour_ >= 1.0f)
        {
            pour_        = 1.0f;
            poured_from_ = nullptr;
        }
    }

    // Each grain through the block, into the mix: its state stays in
    // registers
    std::fill(out[0], out[0] + size, 0.0f);
    std::fill(out[1], out[1] + size, 0.0f);
    Grain::Sums sums;
    for(auto& grain : grains_)
        grain.Render(double(tape_), out[0], out[1], size, sums);
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

    // Stopping: faded over its slowest part
    const float stop_step
        = (std::min(1.0f, tape_ / kStopFade) - stop_gain_) / n;
    const float norm_step = (norm_ - norm_gain_) / n;
    // Dry and wet, equal power, exact at the ends
    const float mix         = std::clamp(p.mix, 0.0f, 1.0f);
    const float mix_angle   = HALFPI_F * mix;
    const float dry         = mix >= 1.0f ? 0.0f : std::cos(mix_angle);
    const float wet         = mix <= 0.0f ? 0.0f : std::sin(mix_angle);
    const float dry_step    = (dry - dry_gain_) / n;
    const float wet_step    = (wet - wet_gain_) / n;
    const float volume_step = (std::clamp(p.volume, 0.0f, 1.0f) - volume_) / n;
    for(size_t i = 0; i < size; i++)
    {
        reverb_dry_ += reverb_dry_step;
        reverb_wet_ += reverb_wet_step;
        norm_gain_ += norm_step;
        stop_gain_ += stop_step;
        const float gain = norm_gain_ * stop_gain_;
        // Into the reverb before the filter
        float reverb_l, reverb_r;
        reverb_.Process(
            out[0][i] * gain, out[1][i] * gain, &reverb_l, &reverb_r);
        float l = out[0][i] * gain;
        float r = out[1][i] * gain;
        filters_[0].Process(l);
        filters_[1].Process(r);
        l += filter_mix
             * ((lowpass ? filters_[0].Low() : filters_[0].High()) - l);
        r += filter_mix
             * ((lowpass ? filters_[1].Low() : filters_[1].High()) - r);

        // The input, always into the ring
        const float in_l = in[0][i];
        const float in_r = in[1][i];
        const float mono = dc_block_.Process(kInputSum * (in_l + in_r));
        ring_.Write(mono);

        out[0][i] = l * reverb_dry_ + reverb_l * reverb_wet_;
        out[1][i] = r * reverb_dry_ + reverb_r * reverb_wet_;
        dry_gain_ += dry_step;
        wet_gain_ += wet_step;
        out[0][i] = out[0][i] * kOutputGain * wet_gain_ + in_l * dry_gain_;
        out[1][i] = out[1][i] * kOutputGain * wet_gain_ + in_r * dry_gain_;
        limiter_.Process(out[0][i], out[1][i]);
        volume_ += volume_step;
        out[0][i] *= volume_;
        out[1][i] *= volume_;
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
        capture_length_ = tapped ? size_t(kTapCaptureSeconds * sample_rate_)
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
        for(auto& grain : grains_)
        {
            if(!grain.IsActive() || grain.Source() != &target)
                continue;
            // Stopped, it would never end: silent, so ended here
            if(tape_ == 0.0f)
            {
                grain.End();
                continue;
            }
            capture_back_ += size;
            return;
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
    pour_ = 0.0f;
    playing_.store(next, std::memory_order_release);
    restart_ = true;
    recordings_.fetch_add(1, std::memory_order_release);
}

void Core::Feed(const float* data, size_t length)
{
    for(size_t i = 0; i < length; i++)
        ring_.Write(data[i]);
}

void Core::Keep(size_t length)
{
    capture_back_   = 0;
    capture_length_ = length;
    capture_        = Capture::Waiting;
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
                      size_t        offset,
                      const Grain*& last)
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
    // Position: most grains near the playhead, fewer further out, within the
    // amount either way, round the selection. The others evenly over their
    // range: size between its ends, exponential, pitch either way.
    double phase = source.phase;
    if(p.rand_pos > 0.0f)
    {
        phase += std::min(p.rand_pos, 1.0f) * Bell();
        phase -= std::floor(phase);
    }
    latch.position = (source.start + phase * source.length) * to_read;
    double length  = 1.0; // of the size
    if(p.rand_size > 0.0f && p.max_grain_ms > p.min_grain_ms)
    {
        const float octaves = std::log2(p.max_grain_ms / p.min_grain_ms);
        const float size    = std::clamp(
            std::log2(p.grain_ms / p.min_grain_ms) / octaves, 0.0f, 1.0f);
        length = std::exp2((Randomise(size, p.rand_size) - size) * octaves);
    }
    latch.inc   = 1.0 / std::max(source.grain_samples * length, 1.0);
    latch.shape = p.rand_shape > 0.0f ? Randomise(p.window_shape, p.rand_shape)
                                      : p.window_shape;
    const float pitch     = std::clamp(p.rand_pitch, -1.0f, 1.0f);
    const float random    = Random();
    float       semitones = random * pitch * kRandPitchSemitones;
    if(pitch < 0.0f)
    {
        const bool away = std::fabs(random) < -pitch * kRandPitchOctaveOdds;
        semitones       = away ? std::copysign(12.0f, random) : 0.0f;
    }
    latch.rate = p.pitch * std::exp2(semitones / 12.0f);
    const bool mirrored
        = p.reverse > 0.0f && 0.5f + 0.5f * Random() < p.reverse;
    if(source.backwards != mirrored)
        latch.rate = -latch.rate;
    // Width with the grains sounding and how much they differ: a lone grain
    // near the centre, most of the others too
    const float width = 0.5f * float(active + 1) * p.spread;
    const float scale = 0.5f - 0.5f * Random();
    latch.pan         = 0.5f + 0.5f * Random() * std::min(width * scale, 1.0f);
    // In phase with the grain before, moved at most a quarter of its length.
    // Not scattered by Spray: those are meant to differ.
    if(p.rand_pos <= 0.0f && last && last->IsActive()
       && last->Source() == latch.buffer && aligns_ >= 1.0f)
    {
        aligns_ -= 1.0f;
        latch.position = Aligner::Align(*latch.buffer,
                                        last->PositionAt(offset),
                                        last->Settings().rate,
                                        latch.position,
                                        latch.rate,
                                        size_t(0.25 / latch.inc));
    }
    grain->Start(latch, offset);
    last = grain;
}

bool Core::Due(Schedule& s, double sched_inc, float random)
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

float Core::Interval(float random)
{
    if(random <= 0.0f)
        return 1.0f;
    // -ln(1 - x) to its fourth term, x even on 0.1..1: nearly exponential
    // (Poisson) without the closest starts and the longest gaps. kMean: of the
    // series, for a mean of 1.
    static constexpr float kMean = 0.883f;
    const float            x     = 0.1f + 0.9f * (0.5f - 0.5f * Random());
    const float            interval
        = x * (1.0f + x * (0.5f + x * (0.3333f + x * 0.25f))) / kMean;
    return 1.0f + std::min(random, 1.0f) * (interval - 1.0f);
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

float Core::Bell()
{
    // Irwin-Hall: bounded, no libm, the same time every call
    float sum = 0.0f;
    for(int i = 0; i < 4; i++)
        sum += Random();
    return 0.25f * sum;
}

} // namespace clessidra
