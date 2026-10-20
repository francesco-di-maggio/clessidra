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
    static constexpr size_t kNumGrains = 8; // poly~ p_grain 8

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
    static constexpr float kGateRampMs = 100.0f; // pattr ramptime @initial 100 in granulator.maxpat

    Grain::Latch _latch(float sel_start, float sel_len, float span);
    float _random(); // -1..1, as noise~

    Buffer _buffer;
    std::array<Grain, kNumGrains> _grains;
    Params _params;
    float _sample_rate = 48000.0f;
    float _gate_inc = 0.0f;
    float _out_gain = 0.0f;
    float _play_phase = 0.0f; // playhead through the selection, 0..1
    uint32_t _seed = 1;
    volatile bool _record = false;
    bool _was_record = false;
};

} // namespace clessidra
