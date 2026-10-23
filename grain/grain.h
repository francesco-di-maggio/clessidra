#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra {

// One grain: a window over a buffer read. Buffer, position, read rate, pan, window
// shape and size are latched at Start(). The window advances at the rate passed to
// Render(), divided by the size.
class Grain {
public:
    struct Latch {
        const Buffer* buffer = nullptr;
        double position = 0.0; // samples
        float rate = 1.0f;     // samples read per output sample
        float pan = 0.5f;      // 0 left .. 1 right
        float shape = 0.5f;    // window: 0 reverse percussive, 0.5 Hann, 1 percussive
        float gain = 1.0f;     // the source's level
        float size = 1.0f;     // of the grain size passed to Render()
    };

    // Once, before any Grain is used.
    static void InitWindow();

    bool IsActive() const { return _active; }
    const Buffer* Source() const { return _latch.buffer; }
    // For displays: the sample being read (not wrapped), the window's progress 0..1, the pan
    double Position() const { return _latch.position + _read; }
    float Progress() const { return float(_phase); }
    float Pan() const { return _latch.pan; }
    // offset: first sample of the grain in the next Render()
    void Start(const Latch& latch, size_t offset);

    // Sums over all grains and samples of a block
    struct Sums {
        float power = 0.0f;        // squared windowed samples, before panning
        float window = 0.0f;       // windows
        float window_power = 0.0f; // squared windows
    };

    // Adds the grain to left and right while active. inc: window phase per sample.
    void Render(double inc, float* left, float* right, size_t size, Sums& sums);

private:
    static constexpr size_t kWindowSize = 512;
    static constexpr float kDecay = 5.0f;    // percussive decay: e^-5 at the end, -43 dB
    static constexpr float kMinPeak = 0.01f; // percussive peak, of the grain
    // Over 0..1: Hann's rise sin^2(pi/2 x), the percussive decay e^-kx cos(pi/2 x)
    static float _rise[kWindowSize + 1];
    static float _decay[kWindowSize + 1];
    static float _lookup(const float* table, float x);

    Latch _latch;
    double _phase = 0.0;
    double _read = 0.0; // samples read since Start()
    size_t _offset = 0;
    float _gain_l = 0.0f;
    float _gain_r = 0.0f;
    // Window: rises to its peak, then decays as Hann blended to the percussive decay
    float _peak = 0.5f;      // of the grain
    float _rise_scale = 2.0f; // 1 / _peak
    float _fall_scale = 2.0f; // 1 / (1 - _peak)
    float _percussive = 0.0f;
    bool _mirrored = false; // percussive window reversed
    bool _active = false;
};

} // namespace clessidra
