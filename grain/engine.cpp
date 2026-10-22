#include "engine.h"

#include "Utility/dsp.h"

#include <algorithm>
#include <cmath>

namespace clessidra {

void Engine::Init(float sample_rate, float* ring, size_t ring_size, float* const* buffers, size_t buffer_size) {
    _sample_rate = sample_rate;
    _ring.Init(ring, ring_size, sample_rate);
    _ring_size = ring_size;
    for (size_t b = 0; b < kNumBuffers; b++) _buffers[b].Init(buffers[b], buffer_size, sample_rate);
    _dc_block.Init(sample_rate);
    _reverb.Init(sample_rate);
    _reverb.SetLpFreq(kReverbLowpassHz);
    _limiter.Init(sample_rate);

    Grain::InitWindow();
}

void Engine::Process(const float* const* in, float** out, size_t size) {
    _record_input(in, size);

    const Params p = _params;
    const double len = double(_buffers[_playing].Length());
    const double sel_start = p.start * len;
    const double sel_len = std::max(p.length * len, 1.0);
    const double place_len = std::max(sel_len / kNumPlaces, 1.0);
    // A grain reads about grain_samples: no longer than the recording
    const double grain_samples = std::clamp(p.grain_ms * 0.001 * _sample_rate, 1.0, std::max(len, 1.0));
    const double grain_inc = p.pitch / grain_samples;

    // Freeze holds every source where it is; position moves them from there
    if (p.freeze && !_was_freeze) {
        for (auto& stream : _streams) stream.frozen = stream.phase;
        _freeze_position = p.position;
    }
    _was_freeze = p.freeze;
    const double scrub = double(p.position) - double(_freeze_position);

    struct Span {
        double start, length;
        double play_inc;  // phase per sample
        double sched_inc; // overlap starts per grain duration
    };
    std::array<Span, kNumStreams> spans;
    for (size_t s = 0; s < kNumStreams; s++) {
        const bool playhead = s == 0;
        const float overlap = std::clamp(playhead ? p.overlap : p.place_overlap[s - 1], 0.0f, kMaxOverlap);
        const double length = playhead ? sel_len : place_len;
        spans[s] = { playhead ? sel_start : sel_start + double(s - 1) * sel_len / kNumPlaces, length,
                     p.speed / length, grain_inc * overlap };

        Stream& stream = _streams[s];
        const bool on = overlap > 0.0f;
        // A place starts at its edge in the playing direction, with a grain at once.
        // The playhead keeps running.
        if (on && !stream.on && !playhead) {
            stream.phase = p.speed < 0.0f ? std::nextafter(1.0, 0.0) : 0.0;
            stream.frozen = stream.phase - scrub;
            stream.sched_phase = stream.sched_next;
        }
        stream.on = on || playhead;
        if (p.freeze) {
            stream.phase = stream.frozen + scrub;
            stream.phase -= std::floor(stream.phase);
        }
    }

    // Grain starts in this block, at the sample they fall on. A voice that ends in this
    // block is free from the next.
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < kNumStreams; s++) {
            Stream& stream = _streams[s];
            const Span& span = spans[s];
            if (!stream.on) continue;
            if (!p.freeze) {
                stream.phase += span.play_inc;
                stream.phase -= std::floor(stream.phase);
            }

            stream.sched_phase += span.sched_inc;
            if (stream.sched_phase >= stream.sched_next) {
                stream.sched_phase -= stream.sched_next;
                _trigger(span.start, span.length, stream.phase, i);
                stream.sched_next = _interval(p.jitter);
                // At most one start per sample
                stream.sched_phase = std::min(stream.sched_phase, double(stream.sched_next));
            }
        }
    }

    // Each grain through the block: its state stays in registers
    std::fill(out[0], out[0] + size, 0.0f);
    std::fill(out[1], out[1] + size, 0.0f);
    Grain::Sums sums;
    for (auto& grain : _grains) grain.Render(grain_inc, out[0], out[1], size, sums);
    float power_sum = 0.0f;
    for (size_t i = 0; i < size; i++) power_sum += out[0][i] * out[0][i] + out[1][i] * out[1][i];

    const float n = float(size);
    _grain_level = sums.window / n;
    _normalise(sums.power / n, power_sum / n, sums.window_power / n, size);

    // ReverbSc, measured with a 10 kHz lowpass: T60 = 3 * kLoop / -log10(feedback) within
    // 20 %, output gain kGain / sqrt(1 - feedback^2) within 1.3 dB from feedback 0.5 to
    // 0.98. The inverse gain keeps the reverb at the level of the grains.
    static constexpr float kLoop = 0.06f; // seconds
    static constexpr float kGain = 0.29f;
    const float reverb = std::clamp(p.reverb, 0.0f, 1.0f);
    const float decay = kReverbDecayMin * std::pow(kReverbDecayMax / kReverbDecayMin, reverb);
    const float feedback = std::pow(10.0f, -3.0f * kLoop / decay);
    _reverb.SetFeedback(feedback);
    const float reverb_gain = std::sqrt(1.0f - feedback * feedback) / kGain;
    const float reverb_angle = 0.5f * float(M_PI) * reverb * kReverbMaxAmount;
    const float reverb_dry_step = (std::cos(reverb_angle) - _reverb_dry) / n;
    const float reverb_wet_step = (reverb_gain * std::sin(reverb_angle) - _reverb_wet) / n;

    const float tone_coeff = 1.0f - std::exp(-n / (kToneSeconds * _sample_rate));
    daisysp::fonepole(_tone, std::clamp(p.tone, -1.0f, 1.0f), tone_coeff);
    const float tone = std::fabs(_tone);
    const bool lowpass = _tone < 0.0f;
    const float tone_hz = lowpass ? kLowpassOffHz * std::pow(kLowpassMinHz / kLowpassOffHz, tone)
                                  : kHighpassOffHz * std::pow(kHighpassMaxHz / kHighpassOffHz, tone);
    const float tone_q = kToneQMin + tone * (kToneQMax - kToneQMin);
    const float tone_mix = std::min(1.0f, tone / kToneFade);
    for (auto& filter : _tone_filter) filter.Set(tone_hz, tone_q, _sample_rate);

    const float angle = 0.5f * float(M_PI) * std::clamp(p.mix, 0.0f, 1.0f);
    const float wet_step = (p.gain * _norm * std::sin(angle) - _wet_gain) / n;
    const float dry_step = (p.gain * std::cos(angle) - _dry_gain) / n;
    for (size_t i = 0; i < size; i++) {
        _reverb_dry += reverb_dry_step;
        _reverb_wet += reverb_wet_step;
        _wet_gain += wet_step;
        _dry_gain += dry_step;
        float l = out[0][i] * _wet_gain;
        float r = out[1][i] * _wet_gain;
        _tone_filter[0].Process(l);
        _tone_filter[1].Process(r);
        l += tone_mix * ((lowpass ? _tone_filter[0].Low() : _tone_filter[0].High()) - l);
        r += tone_mix * ((lowpass ? _tone_filter[1].Low() : _tone_filter[1].High()) - r);
        float reverb_l, reverb_r;
        _reverb.Process(l, r, &reverb_l, &reverb_r);
        out[0][i] = l * _reverb_dry + reverb_l * _reverb_wet + in[0][i] * _dry_gain;
        out[1][i] = r * _reverb_dry + reverb_r * _reverb_wet + in[1][i] * _dry_gain;
        _limiter.Process(out[0][i], out[1][i]);
    }
}

void Engine::_record_input(const float* const* in, size_t size) {
    for (size_t i = 0; i < size; i++) _ring.Write(_dc_block.Process(0.5f * (in[0][i] + in[1][i])));

    const bool record = _record;
    if (record && !_was_record) _held = 0;
    if (record) _held += size;
    // Release: a new capture replaces one in progress
    if (!record && _was_record) {
        const size_t tap = size_t(kTapSeconds * _sample_rate);
        const size_t pre_roll = size_t(kPreRollSeconds * _sample_rate);
        _capture_back = _held < tap ? _held : 0;
        _capture_length = _held < tap ? _buffers[0].Capacity() : _held + pre_roll;
        _capture = Capture::Waiting;
    }
    _was_record = record;

    Buffer& target = _buffers[1 - _playing];
    if (_capture == Capture::Waiting) {
        // Grains started before the last swap can still read it. Only with swaps closer
        // than the longest grain.
        for (const auto& grain : _grains) {
            if (grain.IsActive() && grain.Source() == &target) {
                _capture_back += size;
                return;
            }
        }
        // Waited long: the oldest part is lost to the writing on
        const size_t margin = size_t(kCopyMarginSeconds * _sample_rate);
        const size_t room = _ring_size > _capture_back + margin ? _ring_size - _capture_back - margin : 0;
        _ring.Select(_capture_back, std::min({ _capture_length, room, target.Capacity() }));
        _capture = Capture::Copying;
    }
    if (_capture == Capture::Copying) {
        const Ring::Step step = _ring.Copy(target, kCopyPerBlock);
        if (step == Ring::Step::Pending) return;
        _capture = Capture::Idle;
        if (step == Ring::Step::Done) _playing = 1 - _playing;
    }
}

void Engine::_normalise(float power_grains, float power_sum, float window_power, size_t size) {
    const float coeff = 1.0f - std::exp(-float(size) / (kNormSeconds * _sample_rate));
    daisysp::fonepole(_power_grains, power_grains, coeff);
    daisysp::fonepole(_power_sum, power_sum, coeff);
    daisysp::fonepole(_window_power, window_power, coeff);

    // Uncorrelated grains each carry the input's power times their squared window,
    // split over two channels by the equal-power pan: sqrt(2 / window power) gives
    // each channel the input's power. At least 1: a lone grain peaks at the input level.
    const float overlap_gain = std::sqrt(2.0f / std::max(_window_power, 1.0f));

    // Uncorrelated grains: the sum has the power of the grains, 1. Correlated grains
    // add in amplitude: up to 8 dB above that power at overlap 8, 11 dB at 16.
    // Never above 1: grains that cancel stay quiet. Silence: hold the last value.
    if (_power_sum >= 1e-10f) _coherence_gain = std::min(1.0f, std::sqrt(_power_grains / _power_sum));

    // 0.9 headroom
    _norm = 0.9f * overlap_gain * _coherence_gain;
}

void Engine::_trigger(double start, double length, double phase, size_t offset) {
    const Params& p = _params;
    const Buffer& buffer = _buffers[_playing];
    if (buffer.Length() == 0) return; // nothing recorded: no grains

    Grain* grain = nullptr;
    for (auto& g : _grains) {
        if (!g.IsActive()) {
            grain = &g;
            break;
        }
    }
    if (!grain) return;

    Grain::Latch latch;
    latch.buffer = &buffer;
    latch.position = start + phase * length + _random() * p.rand_pos * length * 0.5;
    latch.rate = p.pitch * (1.0f + _random() * p.rand_pitch);
    // Grains read in the playhead's direction
    if (!p.freeze && p.speed < 0.0f) latch.rate = -latch.rate;
    latch.pan = 0.5f + _random() * p.spread * 0.5f;
    latch.shape = p.window_shape;
    grain->Start(latch, offset);
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
