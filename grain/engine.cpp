#include "engine.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

void Engine::Init(float sample_rate, float* buffer, size_t buffer_size) {
    _sample_rate = sample_rate;
    _buffer.Init(buffer, buffer_size, sample_rate);

    Grain::InitWindow();
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
    // A grain reads about grain_samples: no longer than the recording
    const double grain_samples = std::clamp(p.grain_ms * 0.001 * _sample_rate, 1.0, std::max(len, 1.0));
    const double grain_inc = p.pitch / grain_samples;
    const float overlap = std::clamp(p.overlap, 0.0f, kMaxOverlap);
    // overlap starts per grain duration
    const double sched_inc = grain_inc * overlap;

    float power_grains = 0.0f;
    float power_sum = 0.0f;
    float window_power = 0.0f;
    for (size_t i = 0; i < size; i++) {
        _buffer.Write(0.5f * (in[0][i] + in[1][i]));

        if (p.freeze) {
            _play_phase = p.position;
        } else {
            _play_phase += play_inc;
            _play_phase -= std::floor(_play_phase);
        }

        _sched_phase += sched_inc;
        if (_sched_phase >= _sched_next) {
            _sched_phase -= _sched_next;
            _trigger(sel_start, sel_len);
            _sched_next = _interval(p.jitter);
            // At most one start per sample
            _sched_phase = std::min(_sched_phase, double(_sched_next));
        }

        Grain::Mix mix;
        for (auto& grain : _grains) grain.Process(_buffer, grain_inc, mix);
        power_grains += mix.power;
        power_sum += mix.left * mix.left + mix.right * mix.right;
        window_power += mix.window_power;
        out[0][i] = mix.left;
        out[1][i] = mix.right;
    }

    const float n = float(size);
    _normalise(power_grains / n, power_sum / n, window_power / n, size);

    const float gain_step = (p.gain * _norm - _gain) / n;
    for (size_t i = 0; i < size; i++) {
        _gain += gain_step;
        out[0][i] *= _gain;
        out[1][i] *= _gain;
    }
}

void Engine::_normalise(float power_grains, float power_sum, float window_power, size_t size) {
    const float coeff = 1.0f - std::exp(-float(size) / (kNormSeconds * _sample_rate));
    _power_grains += coeff * (power_grains - _power_grains);
    _power_sum += coeff * (power_sum - _power_sum);
    _window_power += coeff * (window_power - _window_power);

    // Uncorrelated grains each carry the input's power times their squared window,
    // split over two channels by the equal-power pan: sqrt(2 / window power) gives
    // each channel the input's power. At least 1: a lone grain peaks at the input level.
    const float overlap_gain = std::sqrt(2.0f / std::max(_window_power, 1.0f));

    // Uncorrelated grains: the sum has the power of the grains, 1. Correlated grains
    // add in amplitude: up to 8 dB above that power at overlap 8. Never above 1:
    // grains that cancel stay quiet. Silence: hold the last value.
    if (_power_sum >= 1e-10f) _coherence_gain = std::min(1.0f, std::sqrt(_power_grains / _power_sum));

    // 0.9 headroom
    _norm = 0.9f * overlap_gain * _coherence_gain;
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

float Engine::_interval(float jitter) {
    // Mean 1 at any jitter. Exponential intervals (jitter 1) make Poisson starts.
    const float u = 0.5f - 0.5f * _random(); // (0, 1]
    const float exponential = -std::log(u);
    return 1.0f - jitter + jitter * exponential;
}

float Engine::_random() {
    // xorshift32
    _seed ^= _seed << 13;
    _seed ^= _seed >> 17;
    _seed ^= _seed << 5;
    return float(int32_t(_seed)) * (1.0f / 2147483648.0f);
}

} // namespace clessidra
