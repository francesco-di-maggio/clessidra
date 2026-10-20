#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "buffer.h"
#include "grain.h"

namespace clessidra {

// The instrument's signal path. No hardware code here.
class Engine {
public:
    static constexpr size_t kOverlap = 8;    // grains sounding at once, steady state
    static constexpr size_t kNumVoices = 16; // above kOverlap: random starts (flow) can cluster

    struct Params {
        float start = 0.0f;      // selection start, 0..1 of the recording
        float length = 1.0f;     // selection length, 0..1 of the recording
        float speed = 1.0f;      // playhead speed, 1 = real time, negative = reverse
        float grain_ms = 100.0f; // grain size
        float pitch = 1.0f;      // grain playback rate
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
    float _random(); // uniform -1..1

    Buffer _buffer;
    std::array<Grain, kNumVoices> _grains;
    Params _params;
    float _sample_rate = 48000.0f;
    float _out_gain = 0.0f;
    float _gain = 0.0f;       // output gain at the end of the last block
    // Running positions in double: float steps reach 1/8 sample at 30 s
    double _play_phase = 0.0;  // playhead through the selection, 0..1
    double _sched_phase = 0.0; // one grain start per cycle
    uint32_t _seed = 1;
    volatile bool _record = false;
    bool _was_record = false;
};

} // namespace clessidra
