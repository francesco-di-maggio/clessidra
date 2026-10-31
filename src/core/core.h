#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <Effects/reverbsc.h>
#include <Utility/dcblock.h>
#include "grain.h"
#include "buffer.h"
#include "limiter.h"
#include "playhead.h"
#include "svf.h"

namespace clessidra
{
// The instrument's signal path. No hardware code here.
class Core
{
  public:
    Core()            = default;
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;

    // grains per second, three octaves above middle C
    static constexpr float  kMaxDensity = 130.81278f;
    static constexpr size_t kNumVoices  = 32;
    // Voices kept free: over this many sounding, the furthest along fades out,
    // so a new grain always starts
    static constexpr size_t kSafetyVoices = 8;
    static constexpr float  kFadeSeconds  = 0.005f;
    // the furthest a grain's pitch goes at Params.rand_pitch 1
    static constexpr float kRandPitchSemitones = 24.0f;
    // One jack or two: -3 dB on one, +3 dB on two in phase, not -6 and 0
    static constexpr float kInputSum = 0.7071f;
    // +6 dB after the grains, into the limiter: at equal power they sound
    // softer
    static constexpr float kOutputGain = 2.0f;
    // smoothing of the gain measurements
    static constexpr float kNormSeconds = 0.005f;
    // at most: each grain from half to twice the size
    static constexpr float kSizeVariationOctaves = 1.0f;
    // Reverb on the grains: equal-power crossfade up to kReverbMaxAmount, decay
    // (T60) rising with it, exponentially
    static constexpr float kReverbMaxAmount = 0.5f;  // equal parts
    static constexpr float kReverbDecayMin  = 0.5f;  // seconds
    static constexpr float kReverbDecayMax  = 16.0f; // seconds
    // damping in the reverb's loop
    static constexpr float kReverbLowpassHz = 10000.0f;
    // Tone on the grains, before the reverb. From off, exponential to the ends;
    // resonance rising with it from flat (Butterworth). Fades in over the first
    // kToneFade of each side.
    static constexpr float kLowpassOffHz  = 20000.0f;
    static constexpr float kLowpassMinHz  = 200.0f;
    static constexpr float kHighpassOffHz = 20.0f;
    static constexpr float kHighpassMaxHz = 2000.0f;
    static constexpr float kToneQMin      = 0.707f;
    static constexpr float kToneQMax      = 1.5f;
    static constexpr float kToneFade      = 0.1f;
    static constexpr float kToneSeconds   = 0.01f; // smoothing of Params.tone
    // How far behind the write point the sound is read while it comes in
    static constexpr float kStartLagSeconds = 0.05f;

    struct Params
    {
        // Selection, 0..1 of the sound. start: how far back from the held
        // end its end lies. Grains that follow the input read the same length
        // behind the write point.
        float start  = 0.0f;
        float length = 1.0f;
        float speed  = 1.0f; // 1 = real time, negative = reverse
        // where the playhead moves, in selections from its start: a slice can
        // lie outside the selection
        float slice_start = 0.0f;
        float slice_end   = 1.0f;
        // the same, for the grains that follow the input
        float live_slice_start = 0.0f;
        float live_slice_end   = 1.0f;
        // what the playhead does at the end of the slice
        Playhead::End end = Playhead::End::Loop;
        bool  freeze   = false; // the playhead held where it was, speed ignored
        float grain_ms = 100.0f; // grain size
        // 0..1: each grain's size random up to kSizeVariationOctaves either way
        float size_variation = 0.0f;
        float pitch          = 1.0f; // grain playback rate
        // grains per second on average, up to kMaxDensity
        float density = 80.0f;
        float jitter  = 0.0f; // 0 periodic .. 1 random (Poisson) grain starts
        // 0..1, position spread: up to half the selection either way, inside
        // the slice
        float rand_pos = 0.0f;
        // 0..1: most grains near the pitch, a few up to kRandPitchSemitones off
        float rand_pitch = 0.0f;
        // 0..1, how much the grains differ: with the grains sounding, the pan
        // width
        float spread = 0.0f;
        // 0 reverse percussive, 0.5 Hann, 1 percussive
        float window_shape = 0.5f;
        float reverb = 0.0f; // 0 none .. 1 most and longest, on the grains
        float tone   = 0.0f; // -1 lowpass .. 0 off .. 1 highpass, on the grains
        // 0 the buffer sealed .. 1 only the input: what replaces the sound as
        // the write point passes, and the share of grains that follow it
        float fill = 0.0f;
        // gain on the input, before the recording and the dry signal
        float input  = 1.0f;
        float output = 1.0f; // gain after the limiter, 0..1
    };

    // buffer: buffer_size floats, allocated by the caller
    void Init(float sample_rate, float* buffer, size_t buffer_size);
    void Process(const float* const* in, float** out, size_t size);

    // Main loop. A block sees one set whole: written to the slot not in use,
    // then published. The audio callback interrupts this and never the reverse.
    void SetParams(const Params& p)
    {
        params_           = p;
        const size_t next = 1 - slot_.load(std::memory_order_relaxed);
        slots_[next]      = p;
        slot_.store(next, std::memory_order_release);
    }
    // Main loop, after SetParams(). The playhead from the start of the slice,
    // at the next block. Also after each new recording.
    void Restart() { restart_.store(true, std::memory_order_relaxed); }
    // The playhead stopped at the end, in one-shot
    bool HasEnded() const { return ended_.load(std::memory_order_relaxed); }
    // What has come in, at most the buffer
    size_t SoundSamples() const
    {
        return sound_samples_.load(std::memory_order_relaxed);
    }
    // Where the held sound ends, in samples of the buffer
    size_t HeldEnd() const { return held_end_.load(std::memory_order_relaxed); }
    // Grain windows summed, mean over the last block: 1 = one grain at its peak
    float GrainLevel() const
    {
        return grain_level_.load(std::memory_order_relaxed);
    }

    const Buffer& Sound() const { return buffer_; }
    // For tests, between blocks: the grains and the parameters in use
    const std::array<Grain, kNumVoices>& Grains() const { return grains_; }
    const Params& CurrentParams() const { return params_; }
    double        PlayheadPhase() const { return stream_.phase; }
    double        LivePhase() const { return stream_.live_phase; }

  private:
    // The playhead over the selection, and behind the write point
    struct Stream
    {
        double phase       = 0.0; // through its span, 0..1
        double live_phase  = 0.0;
        double sched_phase = 0.0;  // grain starts, 1 = mean interval
        float  sched_next  = 1.0f; // phase of the next start
    };

    // Where a grain reads
    struct Source
    {
        double start;     // of the span, samples
        double length;    // of the span, samples
        double phase;     // through the span
        double low, high; // of the span, where it may scatter
        bool   backwards;
        double newest; // the newest sound it may reach
        double wrap;   // reads inside this many samples, 0: the whole buffer
    };

    // Starts a grain on a free voice at sample offset of the block
    void Trigger(const Params& p,
                 const Source& source,
                 size_t        offset,
                 double        grain_samples);
    // Updates norm_ from the mean powers of the last block
    void  Normalise(float  power_grains,
                    float  power_sum,
                    float  window_power,
                    size_t size);
    float Interval(float jitter); // to the next start, 1 = mean
    float Random();               // uniform -1..1

    Params                params_; // the last set, main loop only
    std::array<Params, 2> slots_;
    std::atomic<size_t>   slot_{0};
    float                 sample_rate_ = 48000.0f;

    // The sound. Input: DC in the buffer would sound as thumps at the grain
    // rate
    daisysp::DcBlock dc_block_;
    Buffer           buffer_;
    size_t           write_ = 0;
    float fill_gain_        = 0.0f;  // Params.fill at the end of the last block
    bool  filled_           = false; // sound has come in
    bool  wrapped_          = false; // the write point has gone all the way
    bool  was_writing_      = false;
    std::atomic<size_t> sound_samples_{0};
    std::atomic<size_t> held_end_{0};

    // Grains. Running positions in double: float steps reach 1/32 sample at 8
    // s.
    std::array<Grain, kNumVoices> grains_;
    Stream                        stream_;
    Playhead                      playhead_;
    Playhead                      live_playhead_;
    std::atomic<bool>             restart_{false};
    std::atomic<bool>             ended_{false};
    uint32_t                      seed_ = 1;
    std::atomic<float>            grain_level_{0.0f};

    // Level. Mean powers, smoothed: grains taken separately, their stereo sum,
    // their windows.
    float power_grains_ = 0.0f;
    float power_sum_    = 0.0f;
    float window_power_ = 0.0f;
    // brings a correlated sum down to the power of its grains
    float coherence_gain_ = 1.0f;
    float norm_           = 0.0f; // grain gain

    // Tone, reverb, output
    std::array<Svf, 2> tone_filter_;
    float              tone_ = 0.0f; // Params.tone, smoothed
    daisysp::ReverbSc  reverb_;
    Limiter            limiter_;
    // Gains at the end of the last block
    float norm_gain_   = 0.0f;
    float reverb_dry_  = 1.0f;
    float reverb_wet_  = 0.0f;
    float input_gain_  = 1.0f;
    float output_gain_ = 1.0f;
};

} // namespace clessidra
