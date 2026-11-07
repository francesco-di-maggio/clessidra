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

    // grains per second, Beads' kMaxGrainFrequency
    static constexpr float kMaxDensity = 130.81278f;
    static constexpr float kMinLength
        = 0.01f; // of the sound: the shortest selection
    // Voices. All sounding, a new grain is dropped and counted, as Giri's cloud
    // does: the neck clogs. Grains keep their size.
    static constexpr size_t kNumVoices = 32;
    // the furthest Params.rand_pitch takes a grain's pitch, either way
    static constexpr float kRandPitchSemitones = 12.0f;
    // the quietest Params.rand_gain takes a grain
    static constexpr float kRandGainDb = -18.0f;
    // One jack or two: -3 dB on one, +3 dB on two in phase, not -6 and 0
    static constexpr float kInputSum = 0.7071f;
    // +6 dB after the grains, into the limiter: at equal power they sound
    // softer
    static constexpr float kOutputGain = 2.0f;
    // smoothing of the gain measurements
    static constexpr float kNormSeconds = 0.005f;
    // Reverb on the grains: equal-power crossfade up to kReverbMaxAmount, decay
    // (T60) rising with it, exponentially
    static constexpr float kReverbMaxAmount = 0.5f;  // equal parts
    static constexpr float kReverbDecayMin  = 0.5f;  // seconds
    static constexpr float kReverbDecayMax  = 16.0f; // seconds
    // damping in the reverb's loop
    static constexpr float kReverbLowpassHz = 10000.0f;
    // Mute: the fade of the grains, in and out
    static constexpr float kMuteSeconds = 0.005f;
    // Filter on the grains, before the reverb. From off, exponential to the ends;
    // resonance rising with it from flat (Butterworth). Fades in over the first
    // kFilterFade of each side.
    static constexpr float kLowpassOffHz  = 20000.0f;
    static constexpr float kLowpassMinHz  = 200.0f;
    static constexpr float kHighpassOffHz = 20.0f;
    static constexpr float kHighpassMaxHz = 2000.0f;
    static constexpr float kFilterQMin    = 0.707f;
    static constexpr float kFilterQMax    = 1.5f;
    static constexpr float kFilterFade    = 0.1f;
    static constexpr float kFilterSeconds = 0.01f; // smoothing of Params.filter
    // Recording. The input goes to the ring all the time. On release, a touch
    // shorter than kTapSeconds keeps the kTapCaptureSeconds before it, a longer
    // one the touch from kPreRollSeconds before it, up to the buffers'
    // capacity. Copied to the buffer not playing, which then plays: over
    // kPourSeconds, each new grain reads it with odds rising from 0 to 1, the
    // others the recording before.
    static constexpr size_t kNumBuffers        = 2;
    static constexpr float  kTapSeconds        = 0.3f;
    static constexpr float  kTapCaptureSeconds = 8.0f;
    static constexpr float  kPourSeconds       = 2.0f;
    static constexpr float  kPreRollSeconds    = 0.05f;
    static constexpr size_t kCopyPerSample = 21; // × real time: 30 s in 1.4 s
    // ring written on during the copy
    static constexpr float kCopyMarginSeconds = 0.5f;
    // The sand at the playhead, as it moves. Hold: untouched. Wear: the grains
    // fall back into it, high-passed and soft-clipped, kWearAmount of each
    // sample replaced per pass (the live granulator's feedback). Live: the
    // input replaces it.
    enum class Sand
    {
        Hold,
        Wear,
        Live
    };
    static constexpr float kWearAmount = 0.3f;
    // the longest block the write head follows
    static constexpr size_t kMaxBlock = 256;

    struct Params
    {
        // Selection, 0..1 of the sound: from start, length long, round the end
        // to the beginning past it
        float start  = 0.0f;
        float length = 1.0f;
        float speed  = 1.0f;  // 1 = real time, negative = reverse
        bool  freeze = false; // the playhead held where it was, speed ignored
        // At the end of the selection: stop, turn over, or round again
        End   end      = End::Loop;
        float grain_ms = 100.0f; // grain size
        // the size's range, exponential: where rand_size takes it
        float min_grain_ms = 10.0f;
        float max_grain_ms = 4000.0f;
        float pitch        = 1.0f; // grain playback rate
        // grains per second on average, up to kMaxDensity
        float density = 80.0f;
        // 0 reverse percussive, 0.5 Hann, 1 percussive
        float window_shape = 0.5f;
        // Randomness of each grain, as Beads' attenuverters, -1..1: 0 none.
        // Above 0, each grain moved by the amount towards a random value: an
        // even spread. Below, by a random part of three times it: mostly near,
        // a long tail. Over each one's range: the selection or slice, the
        // sizes, the shapes, kRandPitchSemitones either way.
        float rand_pos   = 0.0f;
        float rand_size  = 0.0f;
        float rand_shape = 0.0f;
        float rand_pitch = 0.0f;
        // The randomness of the starts, -1..1: 0 periodic. Above 0, each
        // interval moved by the amount within 0..2 of the mean: an even
        // spread. Below, towards Beads' random starts: mostly short, a long
        // tail.
        float rand_density = 0.0f;
        // 0..1, how much the grains differ: with the grains sounding, the pan
        // width
        float spread = 0.0f;
        float filter = 0.0f; // -1 lowpass .. 0 off .. 1 highpass, on the grains
        float reverb = 0.0f; // 0 none .. 1 most and longest, on the grains
        // The grains silent, faded over kMuteSeconds; the reverb rings on
        bool  mute = false;
        float mix  = 1.0f; // 0 dry .. 1 wet, equal power
        // gain on the input, before the recording and the dry signal
        float input  = 1.0f;
        float output = 1.0f; // gain after the limiter, 0..1
        Sand  sand   = Sand::Hold;
        // Randomness of each grain's level and reverb send, -1..1 as the
        // others: level down to kRandGainDb, send from none to all
        float rand_gain = 0.0f;
        float rand_send = 0.0f;
    };

    // ring: ring_size floats. buffers: kNumBuffers of buffer_size floats, the
    // longest recording. Allocated by the caller. The ring holds the longest
    // recording, its pre-roll and kCopyMarginSeconds.
    void Init(float        sample_rate,
              float*       ring,
              size_t       ring_size,
              float* const buffers[kNumBuffers],
              size_t       buffer_size);
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
    // Main loop, after SetParams(). The playhead from the start of the
    // selection, at the next block.
    void Restart() { restart_.store(true, std::memory_order_relaxed); }
    // Main loop: one grain at the playhead, at the next block, whatever the
    // density
    void Seed() { seeds_.fetch_add(1, std::memory_order_relaxed); }
    // Main loop: true while the record pad is held
    void SetRecording(bool on) { record_.store(on, std::memory_order_relaxed); }
    // What a tap keeps, seconds, up to the ring's capacity. kTapCaptureSeconds
    // unless set: the browser tries others.
    void SetTapSeconds(float seconds)
    {
        tap_seconds_.store(seconds, std::memory_order_relaxed);
    }
    // New recordings so far: a change means Sound() is a new recording, and the
    // playhead has restarted
    uint32_t Recordings() const
    {
        return recordings_.load(std::memory_order_acquire);
    }
    size_t SoundSamples() const { return Sound().Length(); }
    // Grain windows summed, mean over the last block: 1 = one grain at its peak
    float GrainLevel() const
    {
        return grain_level_.load(std::memory_order_relaxed);
    }

    // The recording playing
    const Buffer& Sound() const
    {
        return buffers_[playing_.load(std::memory_order_acquire)];
    }
    // For tests, between blocks: the grains and the parameters in use
    const std::array<Grain, kNumVoices>& Grains() const { return grains_; }
    const Params& CurrentParams() const { return params_; }
    double        PlayheadPhase() const { return playhead_.Phase(); }
    // Through the selection, what is left ahead of the playhead, 0..1: 0 once
    // it has run out
    float SandLeft() const { return float(playhead_.Left()); }
    // A grain's size in use, ms: grain_ms within the recording and what the
    // voices hold at this density
    float GrainMs() const { return grain_ms_; }
    // Grains dropped so far, all voices sounding: the neck clogged
    uint32_t Dropped() const
    {
        return dropped_.load(std::memory_order_relaxed);
    }

  private:
    // Grain starts
    struct Schedule
    {
        double phase = 0.0;  // 1 = mean interval
        float  next  = 1.0f; // phase of the next start
    };

    // Where a grain reads
    struct Source
    {
        double start;  // of the span, samples
        double length; // of the span, samples
        double phase;  // through the span
        bool   backwards;
    };

    enum class Capture
    {
        Idle,
        Waiting,
        Copying
    };

    // Starts a grain on a free voice at sample offset of the block, or drops
    // it. While a new recording pours in, it may read the one before.
    void StartGrain(const Params& p,
                    const Source& source,
                    const Buffer* sound,
                    size_t        offset);
    // End of Process(), after the ring is written: on release, copies the
    // recording to the buffer not playing once its grains have ended, then
    // plays it
    void UpdateCapture(size_t size);
    // Advances grain starts by one sample; true when a grain starts
    bool Due(Schedule& s, double sched_inc, float rand_density);
    // Updates norm_ from the mean powers of the last block
    void  Normalise(float  power_grains,
                    float  power_sum,
                    float  window_power,
                    size_t size);
    float Interval(float rand_density); // to the next start, 1 = mean
    // A grain's value of x, 0..1, randomised by amount, as Beads
    float Randomise(float x, float amount);
    // How far towards the random value: the amount above 0, below it a
    // random part of three times it
    float Blend(float amount);
    float Random(); // uniform -1..1

    Params                params_; // the last set, main loop only
    std::array<Params, 2> slots_;
    std::atomic<size_t>   slot_{0};
    float                 sample_rate_ = 48000.0f;

    // The sound. Input: DC in the buffer would sound as thumps at the grain
    // rate
    daisysp::DcBlock                dc_block_;
    Ring                            ring_;
    std::array<Buffer, kNumBuffers> buffers_;
    std::atomic<size_t>             playing_{0};
    std::atomic<uint32_t>           dropped_{0};
    // Wear: the grains, high-passed, before they fall back
    daisysp::DcBlock wear_block_;

    std::atomic<uint32_t> recordings_{0};
    std::atomic<bool>     record_{false};
    std::atomic<float>    tap_seconds_{kTapCaptureSeconds};
    bool                  was_record_ = false;
    // While a new recording pours in: the one before, and how far, 0..1
    const Buffer* poured_from_ = nullptr;
    float         pour_        = 1.0f;
    Capture       capture_     = Capture::Idle;
    size_t        held_        = 0; // samples since the touch
    size_t capture_back_   = 0; // from the recording's end to the next write
    size_t capture_length_ = 0;

    // Grains. Running positions in double: float steps reach 1/32 sample at 8
    // s.
    std::array<Grain, kNumVoices> grains_;
    Playhead                      playhead_;
    Schedule                      schedule_;

    std::atomic<bool>     restart_{false};
    std::atomic<uint32_t> seeds_{0};
    uint32_t              seeds_done_ = 0;
    uint32_t              seed_       = 1;
    // The selection's length in samples at the last block: a new length keeps
    // the playhead where it is in the recording
    double             sel_len_  = 0.0;
    float              grain_ms_ = 0.0f;
    std::atomic<float> grain_level_{0.0f};

    // Level. Mean powers, smoothed: grains taken separately, their stereo sum,
    // their windows.
    float power_grains_ = 0.0f;
    float power_sum_    = 0.0f;
    float window_power_ = 0.0f;
    // brings a correlated sum down to the power of its grains
    float coherence_gain_ = 1.0f;
    float norm_           = 0.0f; // grain gain

    // Filter, reverb, output
    std::array<Svf, 2> filters_;
    float              filter_ = 0.0f; // Params.filter, smoothed
    daisysp::ReverbSc  reverb_;
    Limiter            limiter_;
    // Gains at the end of the last block
    float norm_gain_   = 0.0f;
    float mute_gain_   = 1.0f;
    float reverb_dry_  = 1.0f;
    float reverb_wet_  = 0.0f;
    float input_gain_  = 1.0f;
    float output_gain_ = 1.0f;
    float dry_gain_    = 0.0f;
    float wet_gain_    = 1.0f;
};

} // namespace clessidra
