#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "Utility/dcblock.h"
#include "buffer.h"
#include "grain.h"
#include "limiter.h"

namespace clessidra {

// The instrument's signal path. No hardware code here.
class Engine {
public:
    static constexpr float kMaxOverlap = 8.0f;
    static constexpr size_t kNumVoices = 16; // above kMaxOverlap: random starts can cluster
    static constexpr float kNormSeconds = 0.005f; // smoothing of the gain measurements

    struct Params {
        float start = 0.0f;      // selection start, 0..1 of the recording
        float length = 1.0f;     // selection length, 0..1 of the recording
        float speed = 1.0f;      // playhead speed, 1 = real time, negative = reverse
        bool freeze = false;     // playhead held at position, speed ignored
        float position = 0.0f;   // 0..1 of the selection, used while frozen
        float grain_ms = 100.0f; // grain size
        float pitch = 1.0f;      // grain playback rate
        float overlap = 8.0f;    // grains sounding at once on average, up to kMaxOverlap
        float jitter = 0.0f;     // 0 periodic .. 1 random (Poisson) grain starts
        float rand_pos = 0.0f;   // 0..1, position spread as a fraction of the selection
        float rand_pitch = 0.0f; // 0..1, read span spread
        float spread = 0.5f;     // 0..1, stereo spread
        float chance = 1.0f;     // 0..1, probability that a grain plays
        float gain = 1.0f;
    };

    // buffer: buffer_size floats for the recording, allocated by the caller
    void Init(float sample_rate, float* buffer, size_t buffer_size);
    void Process(const float* const* in, float** out, size_t size);

    // Main loop. Applied at the next audio block.
    void SetRecording(bool on) { _record = on; }
    // Main loop. Fields are copied one by one: a block may see a mix of old and new.
    void SetParams(const Params& p) { _params = p; }

    bool IsRecording() const { return _buffer.IsRecording(); }
    float RecordedSeconds() const { return float(_buffer.Length()) / _sample_rate; }

private:
    // Starts a grain on a free voice. All voices busy: the grain is dropped.
    void _trigger(double sel_start, double sel_len);
    // Updates _norm from the mean powers of the last block
    void _normalise(float power_grains, float power_sum, float window_power, size_t size);
    float _interval(float jitter); // to the next start, 1 = mean
    float _random(); // uniform -1..1

    daisysp::DcBlock _dc_block; // input: DC in a recording would sound as thumps at the grain rate
    Buffer _buffer;
    Limiter _limiter;
    std::array<Grain, kNumVoices> _grains;
    Params _params;
    float _sample_rate = 48000.0f;
    float _gain = 0.0f;       // output gain at the end of the last block
    // Mean powers, smoothed: grains taken separately, their stereo sum, their windows
    float _power_grains = 0.0f;
    float _power_sum = 0.0f;
    float _window_power = 0.0f;
    float _coherence_gain = 1.0f; // brings a correlated sum down to the power of its grains
    float _norm = 0.0f;           // output gain before Params.gain
    // Running positions in double: float steps reach 1/8 sample at 30 s
    double _play_phase = 0.0;  // playhead through the selection, 0..1
    double _sched_phase = 0.0; // grain starts, 1 = mean interval
    float _sched_next = 1.0f;  // phase of the next start
    uint32_t _seed = 1;
    volatile bool _record = false;
    bool _was_record = false;
};

} // namespace clessidra
