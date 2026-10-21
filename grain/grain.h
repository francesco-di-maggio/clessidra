#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra {

// One grain: a window over a buffer read. Position, read rate, pan and window shape
// are latched at Start(). The window advances at the rate passed to Process().
class Grain {
public:
    struct Latch {
        double position = 0.0; // samples
        float rate = 1.0f;     // samples read per output sample
        float pan = 0.5f;      // 0 left .. 1 right
        float shape = 0.5f;    // window: 0 near-rectangle, 0.5 sine, 1 Hann
    };

    // Once, before any Grain is used.
    static void InitWindow();

    bool IsActive() const { return _active; }
    // offset: first sample of the grain in the next Render()
    void Start(const Latch& latch, size_t offset);

    // Sums over all grains and samples of a block
    struct Sums {
        float power = 0.0f;        // squared windowed samples, before panning
        float window_power = 0.0f; // squared windows
    };

    // Adds the grain to left and right while active. inc: window phase per sample.
    void Render(const Buffer& buffer, double inc, float* left, float* right, size_t size, Sums& sums);

private:
    static constexpr size_t kWindowSize = 512;
    static float _window[kWindowSize + 1];

    Latch _latch;
    double _phase = 0.0;
    double _read = 0.0; // samples read since Start()
    size_t _offset = 0;
    float _gain_l = 0.0f;
    float _gain_r = 0.0f;
    float _clip = 1.0f; // sine scaled by this, clipped at 1
    float _hann = 0.0f; // blend from sine to sine squared (Hann)
    bool _active = false;
};

} // namespace clessidra
