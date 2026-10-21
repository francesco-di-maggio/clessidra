#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace clessidra {

// Stereo-linked look-ahead peak limiter. The gain is the minimum over the coming
// samples, released exponentially and smoothed by two box filters, so it reaches
// each peak's gain before the peak: no sample leaves above the threshold. Delays the
// audio by the attack time.
class Limiter {
public:
    static constexpr float kThreshold = 0.944f;     // -0.5 dBFS: margin for peaks between samples
    static constexpr float kAttackSeconds = 0.005f;
    static constexpr float kHoldSeconds = 0.015f;
    static constexpr float kReleaseSeconds = 0.04f;
    static constexpr float kMaxSampleRate = 96000.0f;

    void Init(float sample_rate);
    void Process(float& left, float& right);

private:
    static constexpr size_t kMaxBox = size_t(kMaxSampleRate * kAttackSeconds / 2.0f);
    static constexpr size_t kMaxWindow = size_t(kMaxSampleRate * (kAttackSeconds + kHoldSeconds));

    // Minimum of the last length values: monotonic queue in a ring
    class MovingMin {
    public:
        void Init(size_t length);
        float Process(float x);

    private:
        std::array<float, kMaxWindow> _value;
        std::array<uint32_t, kMaxWindow> _time;
        size_t _length = 1;
        size_t _front = 0;
        size_t _count = 0;
        uint32_t _now = 0;
    };

    // Mean of the last length values
    class Box {
    public:
        void Init(size_t length);
        float Process(float x);

    private:
        std::array<float, kMaxBox> _ring;
        size_t _length = 1;
        double _inv_length = 1.0;
        size_t _pos = 0;
        double _sum = 0.0; // double: float sums drift over hours
    };

    MovingMin _min;
    Box _box1;
    Box _box2;
    float _release = 1.0f;
    float _release_coeff = 0.0f;

    std::array<float, 2 * kMaxBox> _delay_l;
    std::array<float, 2 * kMaxBox> _delay_r;
    size_t _delay = 0;
    size_t _delay_pos = 0;
};

} // namespace clessidra
