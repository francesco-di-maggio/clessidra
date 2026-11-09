#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <Effects/reverbsc.h>
#include <Utility/dcblock.h>
#include "grain.h"
#include "buffer.h"
#include "limiter.h"
#include "onsets.h"
#include "playhead.h"
#include "ring.h"
#include "svf.h"

namespace clessidra
{
// Steps a grain's random pitch snaps to
enum class Quantize
{
    Off,
    Semitones,
    Octaves
};

// The instrument's signal path. No hardware code here.
class Core
{
  public:
    Core()            = default;
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;

    // grains per second: a start every 8 samples at 48 kHz, more than the
    // voices hold at the shortest size
    static constexpr float kMaxDensity = 6000.0f;
    static constexpr float kMinLength
        = 0.01f; // of the sound: the shortest selection
    // Voices. All sounding, a new grain is dropped and counted: the neck
    // clogs. Grains keep their size.
    static constexpr size_t kNumVoices = 32;
    // Slices of the selection, each held one its own playhead
    static constexpr size_t kNumSlices = Onsets::kNumSlices;
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
    // Stop, as a tape machine: everything slowed to a halt, or back up to
    // speed, over this; faded out over the slowest kStopFade of it
    static constexpr float kStopSeconds = 0.6f;
    static constexpr float kStopFade    = 0.25f;
    // Filter on the grains, before the reverb. From off, exponential to the
    // ends, resonance rising with it from flat (Butterworth). Fades in over
    // the first kFilterFade of each side.
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
    // the longest block the reverb sends hold
    static constexpr size_t kMaxBlock = 256;

    struct Params
    {
        // Selection, 0..1 of the sound: from start, length long, ending no
        // later than the sound's end
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
        // Randomness of each grain, 0..1: 0 none, 1 anywhere in the range,
        // evenly. Position within the amount around the playhead, round the
        // selection. Size between min_grain_ms and max_grain_ms, exponential.
        // Pitch up to kRandPitchSemitones either way, snapped by quantize.
        float    rand_pos   = 0.0f;
        float    rand_size  = 0.0f;
        float    rand_pitch = 0.0f;
        Quantize quantize   = Quantize::Off;
        // Starts at random intervals, mostly short, a long tail, the same mean.
        // Otherwise periodic.
        bool random_starts = false;
        // 0..1, how much the grains differ: with the grains sounding, the pan
        // width
        float spread = 0.0f;
        float filter = 0.0f; // -1 lowpass .. 0 off .. 1 highpass, on the grains
        float reverb = 0.0f; // 0 none .. 1 most and longest, on the grains
        // The grains silent, faded over kMuteSeconds; the reverb rings on
        bool mute = false;
        // Slowed to a halt as a tape machine, over kStopSeconds; the reverb
        // rings on
        bool  stop = false;
        float mix  = 1.0f; // 0 dry .. 1 wet, equal power
        // gain on the input, before the recording and the dry signal
        float input  = 1.0f;
        float output = 1.0f; // gain after the limiter, 0..1
        // Randomness of each grain's level and reverb send, 0..1 as the
        // others: level down to kRandGainDb, send from none to all
        float rand_gain = 0.0f;
        float rand_send = 0.0f;
        // Slices of the selection, 0..1 of it. Each held one plays its own
        // span from its start, round it, at speed and density, a grain at
        // once. At no density: two grains at once, regular, the slice as it
        // is. While any is held the selection's playhead waits.
        std::array<float, kNumSlices> slice_start{};
        std::array<float, kNumSlices> slice_length{};
        uint32_t                      slices_held = 0; // bit i: slice i
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
    // Main loop, at the next block: the playhead to phase of the selection,
    // 0..1, a grain there at once whatever the density, the next start an
    // interval later
    void Jump(float phase)
    {
        jump_.store(std::clamp(phase, 0.0f, 1.0f), std::memory_order_relaxed);
    }
    // Main loop, at the next block: while frozen, the playhead moved by delta
    // of the selection, round it. Moves add up until then.
    void Scrub(float delta)
    {
        float now = scrub_.load(std::memory_order_relaxed);
        while(!scrub_.compare_exchange_weak(
            now, now + delta, std::memory_order_relaxed))
        {}
    }
    // Main loop: true while the record pad is held
    void SetRecording(bool on) { record_.store(on, std::memory_order_relaxed); }
    // Between blocks, on the audio thread (the browser's file loading): samples
    // into the ring as if they had come in, then the last length of them kept
    // as a recording, as a held P10 released
    void Feed(const float* data, size_t length);
    void Keep(size_t length);
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
    // 1 upright, -1 turned over
    int Heading() const { return playhead_.Heading(); }
    // Through the selection, what is left ahead of the playhead, 0..1: 0 once
    // it has run out
    float SandLeft() const { return float(playhead_.Left()); }
    // A grain's size in use, ms: grain_ms within the recording
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
    bool Due(Schedule& s, double sched_inc, bool random);
    // Updates norm_ from the mean powers of the last block
    void  Normalise(float  power_grains,
                    float  power_sum,
                    float  window_power,
                    size_t size);
    float Interval(bool random); // to the next start, 1 = mean
    // x, 0..1, moved by amount towards an even random value
    float Randomise(float x, float amount);
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

    std::atomic<uint32_t> recordings_{0};
    std::atomic<bool>     record_{false};
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
    std::array<Grain, kNumVoices>    grains_;
    Playhead                         playhead_;
    Schedule                         schedule_;
    std::array<Playhead, kNumSlices> slice_heads_;
    std::array<Schedule, kNumSlices> slice_schedules_;
    uint32_t                         slices_held_ = 0; // at the last block

    bool               restart_ = false; // a new recording: from the top
    std::atomic<float> scrub_{0.0f};
    std::atomic<float> jump_{-1.0f}; // none below 0
    uint32_t           seed_     = 1;
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
    float tape_        = 1.0f; // speed of everything, Params.stop ramping it
    float reverb_dry_  = 1.0f;
    float reverb_wet_  = 0.0f;
    float input_gain_  = 1.0f;
    float output_gain_ = 1.0f;
    float dry_gain_    = 0.0f;
    float wet_gain_    = 1.0f;
};

} // namespace clessidra
