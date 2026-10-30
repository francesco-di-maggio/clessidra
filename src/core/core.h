#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <Effects/reverbsc.h>
#include <Utility/dcblock.h>
#include "grain.h"
#include "limiter.h"
#include "playhead.h"
#include "ring.h"
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
    // smoothing of the gain measurements
    static constexpr float kNormSeconds = 0.005f;
    // +6 dB on the grains against the dry signal: at equal power they sound
    // softer
    static constexpr float kWetGain = 2.0f;
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
    // Recording. The input goes to a ring all the time. On release, a touch
    // shorter than kTapSeconds takes the last kTapKeepSeconds, a longer one the
    // touch from kPreRollSeconds before it; copied to the buffer not playing,
    // which then plays.
    static constexpr size_t kNumBuffers     = 2;
    static constexpr float  kTapSeconds     = 0.3f;
    static constexpr float  kTapKeepSeconds = 8.0f;
    static constexpr float  kPreRollSeconds = 0.05f;
    static constexpr size_t kCopyPerBlock = 1024; // samples: 21 times real time
    // ring written on during the copy
    static constexpr float kCopyMarginSeconds = 0.5f;

    struct Params
    {
        float start  = 0.0f; // selection start, 0..1 of the recording
        float length = 1.0f; // selection length, 0..1 of the recording
        float speed  = 1.0f; // 1 = real time, negative = reverse
        // where the playhead moves, in selections from its start: a slice can
        // lie outside the selection
        float slice_start = 0.0f;
        float slice_end   = 1.0f;
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
        float mix    = 0.5f; // 0 input only .. 1 grains only, equal power
        // recorded: 0 input .. 1 wet (after tone and reverb), equal power
        float resample = 0.0f;
        // gain on the input, before the recording and the dry signal
        float input  = 1.0f;
        float output = 1.0f; // gain after the limiter, 0..1
    };

    // ring: ring_size floats; buffers: kNumBuffers of buffer_size floats, the
    // longest recording. All allocated by the caller. The ring holds the tap,
    // pre-roll and copy margin beyond buffer_size.
    void Init(float         sample_rate,
              float*        ring,
              size_t        ring_size,
              float* const* buffers,
              size_t        buffer_size);
    void Process(const float* const* in, float** out, size_t size);

    // Main loop. Applied at the next audio block: on release, the recording
    // replaces the playing one unless it is silent. Silence at both ends is
    // cut.
    void SetRecording(bool on) { record_.store(on, std::memory_order_relaxed); }
    // Main loop. Fields are copied one by one: a block may see a mix of old and
    // new.
    void SetParams(const Params& p) { params_ = p; }
    // Main loop, after SetParams(). The playhead from the start of the slice,
    // at the next block. Also after each new recording.
    void Restart() { restart_.store(true, std::memory_order_relaxed); }
    // The playhead stopped at the end, in one-shot
    bool HasEnded() const { return ended_.load(std::memory_order_relaxed); }
    // Rises with each new recording
    uint32_t RecordingCount() const
    {
        return recording_count_.load(std::memory_order_relaxed);
    }

    bool IsRecording() const { return record_.load(std::memory_order_relaxed); }
    float RecordedSeconds() const
    {
        return float(buffers_[playing_].Length()) / sample_rate_;
    }
    // Grain windows summed, mean over the last block: 1 = one grain at its peak
    float GrainLevel() const
    {
        return grain_level_.load(std::memory_order_relaxed);
    }

    // The recording playing
    const Buffer& Recording() const { return buffers_[playing_]; }
    // For tests, between blocks: the grains and the parameters in use
    const std::array<Grain, kNumVoices>& Grains() const { return grains_; }
    const Params& CurrentParams() const { return params_; }
    double        PlayheadPhase() const { return stream_.phase; }

  private:
    // The playhead over the selection
    struct Stream
    {
        double phase       = 0.0;  // through its span, 0..1
        double sched_phase = 0.0;  // grain starts, 1 = mean interval
        float  sched_next  = 1.0f; // phase of the next start
    };

    enum class Capture
    {
        Idle,
        Waiting,
        Copying
    };

    // Starts a grain on a free voice at sample offset of the block, reading the
    // span at phase
    void Trigger(const Params& p,
                 double        start,
                 double        length,
                 double        phase,
                 bool          backwards,
                 size_t        offset);
    // Updates norm_ from the mean powers of the last block
    void Normalise(float  power_grains,
                   float  power_sum,
                   float  window_power,
                   size_t size);
    // End of Process(), after the ring is written: on release, copies the
    // recording to the buffer not playing once its grains have ended, then
    // plays it.
    void  UpdateCapture(size_t size);
    float Interval(float jitter); // to the next start, 1 = mean
    float Random();               // uniform -1..1

    Params params_;
    float  sample_rate_ = 48000.0f;

    // Recording
    // input: DC in a recording would sound as thumps at the grain rate
    daisysp::DcBlock                dc_block_;
    Ring                            ring_;
    size_t                          ring_size_ = 0;
    std::array<Buffer, kNumBuffers> buffers_;
    size_t                          playing_ = 0;
    Capture                         capture_ = Capture::Idle;
    size_t                          held_    = 0; // samples since the touch
    // samples from the recording's end to the next write
    size_t            capture_back_   = 0;
    size_t            capture_length_ = 0;
    std::atomic<bool> record_{false};
    bool              was_record_ = false;

    // Grains. Running positions in double: float steps reach 1/32 sample at 8
    // s.
    std::array<Grain, kNumVoices> grains_;
    Stream                        stream_;
    Playhead                      playhead_;
    std::atomic<bool>             restart_{false};
    std::atomic<bool>             ended_{false};
    std::atomic<uint32_t>         recording_count_{0};
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
    float norm_gain_    = 0.0f;
    float wet_gain_     = 0.0f;
    float dry_gain_     = 0.0f;
    float record_input_ = 1.0f;
    float record_wet_   = 0.0f;
    float reverb_dry_   = 1.0f;
    float reverb_wet_   = 0.0f;
    float input_gain_   = 1.0f;
    float output_gain_  = 1.0f;
};

} // namespace clessidra
