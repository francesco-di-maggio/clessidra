#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra {

// One grain: a half-sine window over a buffer read. Position, read rate and pan are
// latched at Start(). The window advances at the rate passed to Process().
class Grain {
public:
    struct Latch {
        double position = 0.0; // samples
        float rate = 1.0f;     // samples read per output sample
        float pan = 0.5f;      // 0 left .. 1 right
    };

    // Once, before any Grain is used.
    static void InitWindow();

    bool IsActive() const { return _active; }
    void Start(const Latch& latch);

    // Adds the grain to left and right while active. inc: window phase per sample.
    void Process(const Buffer& buffer, double inc, float& left, float& right);

private:
    static constexpr size_t kWindowSize = 512;
    static float _window[kWindowSize + 1];

    Latch _latch;
    double _phase = 0.0;
    double _read = 0.0; // samples read since Start()
    float _gain_l = 0.0f;
    float _gain_r = 0.0f;
    bool _active = false;
};

} // namespace clessidra
