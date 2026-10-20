#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra {

// One grain voice: a phasor drives a half-sine window and a buffer read. Position,
// read span, pan and play are latched at each window start.
class Grain {
public:
    struct Latch {
        float position = 0.0f; // samples
        float span = 0.0f;     // samples read over one window
        float pan = 0.5f;      // 0 left .. 1 right
        bool play = false;
    };

    // Once, before any Grain is used.
    static void InitWindow();

    // phase: 0..1 start offset, spreads the voices evenly
    void Init(float phase) { _phase = phase; }

    // Advances the phasor. Returns true when a new window starts: call Start() then.
    bool Advance(float inc);
    void Start(const Latch& latch);

    // gate_inc: per-sample step of the play gate ramp
    void Process(const Buffer& buffer, float gate_inc, float& left, float& right);

private:
    static constexpr size_t kWindowSize = 512;
    static float _window[kWindowSize + 1];

    float _phase = 0.0f;
    float _gate = 0.0f;
    Latch _latch;
    float _gain_l = 0.0f;
    float _gain_r = 0.0f;
};

} // namespace clessidra
