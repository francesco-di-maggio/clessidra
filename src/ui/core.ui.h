#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "common/takeover.h"
#include "core/core.h"
#include "hw/controls.h"
#include "nocopy.h"

namespace clessidra {

// Maps the Simple Touch's controls to the core's parameters, and drives the LED.
// Holding P11 switches the knobs to a second layer, holding P00 to the levels. No
// hardware code: the board and the host tests run the same mapping.
class CoreUI {
public:
    // Pads
    static constexpr size_t kRecordPad = 10; // P10, tap or hold to record
    static constexpr size_t kShiftPad = 11;  // P11, hold for the second knob layer
    static constexpr size_t kStopPad = 2;    // P02, hold to stop the grains
    static constexpr size_t kResetPad = 1;   // P01, tap: all but the levels back to the defaults
    static constexpr size_t kLevelsPad = 0;  // P00, hold for the levels: S36 input, S37 output
    // The core's places, first to last in the selection: pads left to right on the
    // faceplate, top to bottom within a column. Pressure sets each place's density.
    static constexpr size_t kPlacePads[] = { 3, 4, 8, 5, 6, 9, 7 };

    static constexpr float kMaxInput = 4.0f;    // input gain at the top: +12 dB

    // LED
    static constexpr float kGlow = 0.6f;            // brightness while grains play, below the pulse
    static constexpr uint32_t kRecordFlashMs = 125; // on and off while P10 is touched: 4 Hz
    static constexpr uint32_t kPulseMs = 300;       // on at a reset, and when a control reaches its centre and while turned there
    static constexpr float kPulseStep = 0.01f;      // turn within a centre that renews the pulse
    static constexpr float kShowStep = 0.01f;       // second-layer or levels change that counts as turning

    CoreUI();

    // Every control pass. Sets the core's parameters and recording.
    void Process(const Controls& c, Core& core);
    // A knob's second-layer value
    float SecondLayer(size_t knob) const { return _shift[knob % Controls::kNumKnobs]; }
    // The LED's light output, 0..1: brightness cubed, even steps to the eye
    float Led(uint32_t now_ms, const Core& core) const;

private:
    NOCOPY(CoreUI)
    // A value per knob
    struct Layer {
        std::array<Takeover, Controls::kNumKnobs> values;

        float operator[](size_t knob) const { return values[knob].Value(); }
        void Engage(const Controls& c);
        void Process(const Controls& c);
    };

    // A layer's values at their last move, and the knob last turned: what the LED shows
    struct Shown {
        std::array<float, Controls::kNumKnobs> values{};
        size_t knob = 0;

        void Update(const Layer& layer);
    };

    // Sound defaults: main and second layer. Levels kept.
    void _reset(const Controls& c);

    Layer _main;
    Layer _shift;
    Layer _levels;
    Layer* _layer = &_main;
    bool _started = false;
    bool _shifted = false;
    bool _levels_on = false;
    bool _reset_touched = false;
    bool _frozen = false;
    float _speed = 0.5f; // S36 at the freeze
    // LED. Times in ms, shown until they pass.
    uint32_t _pulse_until = 0;
    uint32_t _record_since = 0; // P10 touched
    bool _recording = false;
    Shown _shift_shown; // at power-up: dry/wet
    Shown _levels_shown; // at power-up: output
    // Pitch unison, speed 1, grain size 100 ms, tone off, shape Hann, input unity: at the
    // centre, and the knob when the pulse last started
    static constexpr size_t kNumDetents = 6;
    std::array<bool, kNumDetents> _at_detent{};
    std::array<float, kNumDetents> _detent_knob{};
    bool _detents_ready = false;
};

} // namespace clessidra
