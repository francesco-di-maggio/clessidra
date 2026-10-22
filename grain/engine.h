#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "Effects/reverbsc.h"
#include "Utility/dcblock.h"
#include "buffer.h"
#include "grain.h"
#include "limiter.h"
#include "svf.h"

namespace clessidra {

// The instrument's signal path. No hardware code here.
class Engine {
public:
    static constexpr float kMaxOverlap = 16.0f;
    static constexpr size_t kNumVoices = 32; // above kMaxOverlap: random starts can cluster
    static constexpr float kNormSeconds = 0.005f; // smoothing of the gain measurements
    // The selection divided into equal places, each a grain source of its own
    static constexpr size_t kNumPlaces = 9;
    // Reverb on the grains: equal-power crossfade up to kReverbMaxAmount, decay (T60)
    // rising with it, exponentially
    static constexpr float kReverbMaxAmount = 0.5f; // equal parts
    static constexpr float kReverbDecayMin = 0.5f;  // seconds
    static constexpr float kReverbDecayMax = 16.0f; // seconds
    static constexpr float kReverbLowpassHz = 10000.0f; // damping in the reverb's loop
    // Tone on the grains, before the reverb. From off, exponential to the ends; resonance
    // rising with it from flat (Butterworth). Fades in over the first kToneFade of each side.
    static constexpr float kLowpassOffHz = 20000.0f;
    static constexpr float kLowpassMinHz = 500.0f;
    static constexpr float kHighpassOffHz = 20.0f;
    static constexpr float kHighpassMaxHz = 2000.0f;
    static constexpr float kToneQMin = 0.707f;
    static constexpr float kToneQMax = 1.5f;
    static constexpr float kToneFade = 0.1f;
    static constexpr float kToneSeconds = 0.01f; // smoothing of Params.tone
    // Recording. The input goes to a ring all the time. On release, a touch shorter than
    // kTapSeconds takes what came before it, a longer one the touch from kPreRollSeconds
    // before it; copied to the buffer not playing, which then plays.
    static constexpr size_t kNumBuffers = 2;
    static constexpr float kTapSeconds = 0.3f;
    static constexpr float kPreRollSeconds = 0.05f;
    static constexpr size_t kCopyPerBlock = 2048;    // samples: 42 times real time
    static constexpr float kCopyMarginSeconds = 0.5f; // ring written on during the copy

    struct Params {
        float start = 0.0f;        // selection start, 0..1 of the recording
        float length = 1.0f;       // selection length, 0..1 of the recording
        float speed = 1.0f;        // every source, 1 = real time, negative = reverse
        bool freeze = false;       // every source held where it was, speed ignored
        float position = 0.0f;     // while frozen, moves every source through its span by its change
        float grain_ms = 100.0f;   // grain size
        float pitch = 1.0f;        // grain playback rate
        float overlap = 8.0f;      // playhead: grains sounding at once on average, up to kMaxOverlap
        std::array<float, kNumPlaces> place_overlap{}; // the same per place, 0 = off
        float jitter = 0.0f;       // 0 periodic .. 1 random (Poisson) grain starts
        float rand_pos = 0.0f;     // 0..1, position spread: up to half the source's span
        float rand_pitch = 0.0f;   // 0..1, read span spread
        float spread = 0.5f;       // 0..1, stereo spread
        float window_shape = 0.5f; // 0 near-rectangle, 0.5 sine, 1 Hann
        float reverb = 0.0f;       // 0 none .. 1 most and longest, on the grains
        float tone = 0.0f;         // -1 lowpass .. 0 off .. 1 highpass, on the grains
        float mix = 0.5f;          // 0 input only .. 1 grains only, equal power
        float gain = 1.0f;
    };

    // ring: ring_size floats; buffers: kNumBuffers of buffer_size floats, the longest
    // recording. All allocated by the caller. The ring holds the tap, pre-roll and
    // copy margin beyond buffer_size.
    void Init(float sample_rate, float* ring, size_t ring_size, float* const* buffers, size_t buffer_size);
    void Process(const float* const* in, float** out, size_t size);

    // Main loop. Applied at the next audio block: on release, the recording replaces
    // the playing one unless it is silent. Silence at both ends is cut.
    void SetRecording(bool on) { _record = on; }
    // Main loop. Fields are copied one by one: a block may see a mix of old and new.
    void SetParams(const Params& p) { _params = p; }

    bool IsRecording() const { return _record; }
    float RecordedSeconds() const { return float(_buffers[_playing].Length()) / _sample_rate; }
    // Grain windows summed, mean over the last block: 1 = one grain at its peak
    float GrainLevel() const { return _grain_level; }

private:
    // A source of grains: the playhead over the selection, or a place
    struct Stream {
        double phase = 0.0;       // through its span, 0..1
        double frozen = 0.0;      // phase at the freeze, or at a place's start while frozen
        double sched_phase = 0.0; // grain starts, 1 = mean interval
        float sched_next = 1.0f;  // phase of the next start
        bool on = false;
    };
    static constexpr size_t kNumStreams = 1 + kNumPlaces; // playhead first

    // Starts a grain on a free voice at sample offset of the block, reading the span at
    // phase. All voices busy: dropped.
    void _trigger(double start, double length, double phase, size_t offset);
    // Updates _norm from the mean powers of the last block
    void _normalise(float power_grains, float power_sum, float window_power, size_t size);
    float _interval(float jitter); // to the next start, 1 = mean
    float _random(); // uniform -1..1

    daisysp::DcBlock _dc_block; // input: DC in a recording would sound as thumps at the grain rate
    // Process() only: writes the input to the ring; on release, copies the recording to
    // the buffer not playing once its grains have ended, then plays it.
    void _record_input(const float* const* in, size_t size);

    enum class Capture { Idle, Waiting, Copying };

    Ring _ring;
    size_t _ring_size = 0;
    std::array<Buffer, kNumBuffers> _buffers;
    size_t _playing = 0;
    Capture _capture = Capture::Idle;
    size_t _held = 0;            // samples since the touch
    size_t _capture_back = 0;    // samples from the recording's end to the next write
    size_t _capture_length = 0;
    daisysp::ReverbSc _reverb;
    std::array<Svf, 2> _tone_filter;
    Limiter _limiter;
    std::array<Grain, kNumVoices> _grains;
    Params _params;
    float _sample_rate = 48000.0f;
    // Output gains at the end of the last block
    float _wet_gain = 0.0f;
    float _dry_gain = 0.0f;
    float _reverb_dry = 1.0f;
    float _reverb_wet = 0.0f;
    float _tone = 0.0f; // Params.tone, smoothed
    // Mean powers, smoothed: grains taken separately, their stereo sum, their windows
    float _power_grains = 0.0f;
    float _power_sum = 0.0f;
    float _window_power = 0.0f;
    float _coherence_gain = 1.0f; // brings a correlated sum down to the power of its grains
    float _norm = 0.0f;           // output gain before Params.gain
    // Running positions in double: float steps reach 1/8 sample at 30 s
    std::array<Stream, kNumStreams> _streams;
    uint32_t _seed = 1;
    volatile float _grain_level = 0.0f;
    float _freeze_position = 0.0f; // Params.position at the freeze
    volatile bool _record = false;
    bool _was_record = false;
    bool _was_freeze = false;
};

} // namespace clessidra
