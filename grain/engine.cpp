#include "engine.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

void Engine::Init(float sample_rate, float* buffer, size_t buffer_size) {
    _sample_rate = sample_rate;
    _buffer.Init(buffer, buffer_size, sample_rate);

    Grain::InitWindow();
    // 0.9 headroom; sqrt(2) offsets the equal-power pan at centre;
    // 1 / sqrt(voices) keeps the sum of uncorrelated grains near unity
    _out_gain = 0.9f * std::sqrt(2.0f) / std::sqrt(float(kOverlap));
}

void Engine::Process(const float* const* in, float** out, size_t size) {
    // Edges only: a recording that fills the buffer stays stopped while P10 is held.
    const bool record = _record;
    if (record && !_was_record) _buffer.StartRecording();
    if (!record && _was_record) _buffer.StopRecording();
    _was_record = record;

    const Params p = _params;
    const double len = double(_buffer.Length());
    const double sel_start = p.start * len;
    const double sel_len = std::max(p.length * len, 1.0);
    const double play_inc = p.speed / sel_len;
    const double grain_samples = std::max(p.grain_ms * 0.001 * _sample_rate, 1.0);
    const double grain_inc = p.pitch / grain_samples;
    // One start every 1 / kOverlap of a grain
    const double sched_inc = grain_inc * kOverlap;
    const float gain_step = (p.gain * _out_gain - _gain) / float(size);

    for (size_t i = 0; i < size; i++) {
        _buffer.Write(0.5f * (in[0][i] + in[1][i]));

        if (p.freeze) {
            _play_phase = p.position;
        } else {
            _play_phase += play_inc;
            _play_phase -= std::floor(_play_phase);
        }

        _sched_phase += sched_inc;
        if (_sched_phase >= 1.0) {
            _sched_phase -= std::floor(_sched_phase);
            _trigger(sel_start, sel_len);
        }

        float left = 0.0f;
        float right = 0.0f;
        for (auto& grain : _grains) grain.Process(_buffer, grain_inc, left, right);

        _gain += gain_step;
        out[0][i] = left * _gain;
        out[1][i] = right * _gain;
    }
}

void Engine::_trigger(double sel_start, double sel_len) {
    const Params& p = _params;
    // Uniform -1..1 above 1 - 2 * chance: probability chance
    if (_random() <= 1.0f - 2.0f * p.chance) return;

    Grain* grain = nullptr;
    for (auto& g : _grains) {
        if (!g.IsActive()) {
            grain = &g;
            break;
        }
    }
    if (!grain) return;

    Grain::Latch latch;
    latch.position = sel_start + _play_phase * sel_len + _random() * p.rand_pos * sel_len * 0.5;
    latch.rate = p.pitch * (1.0f + _random() * p.rand_pitch);
    // Grains read in the playhead's direction
    if (!p.freeze && p.speed < 0.0f) latch.rate = -latch.rate;
    latch.pan = 0.5f + _random() * p.spread * 0.5f;
    grain->Start(latch);
}

float Engine::_random() {
    // xorshift32
    _seed ^= _seed << 13;
    _seed ^= _seed >> 17;
    _seed ^= _seed << 5;
    return float(int32_t(_seed)) * (1.0f / 2147483648.0f);
}

} // namespace clessidra
