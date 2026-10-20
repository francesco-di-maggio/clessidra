#include "engine.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

void Engine::Init(float sample_rate, float* buffer, size_t buffer_size) {
    _sample_rate = sample_rate;
    _buffer.Init(buffer, buffer_size, sample_rate);

    Grain::InitWindow();
    for (size_t i = 0; i < kNumGrains; i++) {
        _grains[i].Init(float(i) / float(kNumGrains));
    }
    _gate_inc = 1000.0f / (kGateRampMs * sample_rate);
    // gain * 0.9 in granulator, * 1.414 / sqrt(voices) in p_grain
    _out_gain = 0.9f * std::sqrt(2.0f) / std::sqrt(float(kNumGrains));
}

void Engine::Process(const float* const* in, float** out, size_t size) {
    // Edges only: a recording that fills the buffer stays stopped while P10 is held.
    const bool record = _record;
    if (record && !_was_record) _buffer.StartRecording();
    if (!record && _was_record) _buffer.StopRecording();
    _was_record = record;

    const Params p = _params;
    const float len = float(_buffer.Length());
    const float sel_start = p.start * len;
    const float sel_len = std::max(p.length * len, 1.0f);
    const float play_inc = p.speed / sel_len;
    const float grain_samples = std::max(p.grain_ms * 0.001f * _sample_rate, 1.0f);
    const float grain_inc = p.pitch / grain_samples;
    const float gain = p.gain * _out_gain;

    for (size_t i = 0; i < size; i++) {
        _buffer.Write(0.5f * (in[0][i] + in[1][i]));

        _play_phase += play_inc;
        _play_phase -= std::floor(_play_phase);

        float left = 0.0f;
        float right = 0.0f;
        for (auto& grain : _grains) {
            if (grain.Advance(grain_inc)) grain.Start(_latch(sel_start, sel_len, grain_samples));
            grain.Process(_buffer, _gate_inc, left, right);
        }

        out[0][i] = left * gain;
        out[1][i] = right * gain;
    }
}

Grain::Latch Engine::_latch(float sel_start, float sel_len, float span) {
    const Params& p = _params;
    Grain::Latch latch;
    latch.position = sel_start + _play_phase * sel_len + _random() * p.rand_pos * sel_len * 0.5f;
    latch.span = span * (1.0f + _random() * p.rand_pitch);
    latch.pan = 0.5f + _random() * p.spread * 0.5f;
    // chance 0..100 % scaled to a 1..-1 threshold in granulator
    latch.play = _random() > 1.0f - 2.0f * p.chance;
    return latch;
}

float Engine::_random() {
    // xorshift32
    _seed ^= _seed << 13;
    _seed ^= _seed >> 17;
    _seed ^= _seed << 5;
    return float(int32_t(_seed)) * (1.0f / 2147483648.0f);
}

} // namespace clessidra
