#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "../grain/engine.h"

namespace clessidra {

// The Simple Touch panel as plain values: what the board reads, or what a page sends.
struct Controls {
    enum Knob : size_t { S30, S31, S32, S33, S34, S35, S36, S37, kNumKnobs }; // S36, S37: faders
    enum Switch : size_t { S09, S07, kNumSwitches };
    enum Position : int { Centre, Up, Down }; // as libDaisy's Switch3
    static constexpr size_t kNumPads = 12; // P00-P11

    std::array<float, kNumKnobs> knobs{};          // 0..1
    std::array<Position, kNumSwitches> switches{}; // centre
    std::array<bool, kNumPads> touched{};
    std::array<float, kNumPads> pressure{};        // 0..1
    uint32_t now_ms = 0;
};

// Maps the Simple Touch's controls to the engine's parameters, and drives the LED.
// Holding P11 switches the knobs to a second layer, holding P00 to the levels. No
// hardware code: the board and the web page run the same mapping.
class ClessidraUI {
public:
    // Pads
    static constexpr size_t kRecordPad = 10; // P10, tap or hold to record
    static constexpr size_t kShiftPad = 11;  // P11, hold for the second knob layer
    static constexpr size_t kStopPad = 2;    // P02, hold to stop the grains
    static constexpr size_t kResetPad = 1;   // P01, tap: all but the levels back to the defaults
    static constexpr size_t kLevelsPad = 0;  // P00, hold for the levels: S36 input, S37 output
    // The engine's places, first to last in the selection: pads left to right on the
    // faceplate, top to bottom within a column. Pressure sets each place's density.
    static constexpr size_t kPlacePads[] = { 3, 4, 8, 5, 6, 9, 7 };

    static constexpr float kMaxInput = 4.0f;    // input gain at the top: +12 dB

    // LED
    static constexpr float kGlow = 0.6f;            // brightness while grains play, below the pulse
    static constexpr uint32_t kRecordFlashMs = 125; // on and off while P10 is touched: 4 Hz
    static constexpr uint32_t kPulseMs = 300;       // on at a reset, and when a control reaches its centre and while turned there
    static constexpr float kPulseStep = 0.01f;      // turn within a centre that renews the pulse
    static constexpr float kShowStep = 0.01f;       // second-layer or levels change that counts as turning

    ClessidraUI();

    // Every control pass. Sets the engine's parameters and recording.
    void Process(const Controls& c, Engine& engine);
    // For panels with a control per function: sets a knob's second-layer value, as if
    // turned with P11 held
    void SetSecondLayer(size_t knob, float value) { _shift.values[knob % Controls::kNumKnobs] = Takeover(value); }
    float SecondLayer(size_t knob) const { return _shift[knob % Controls::kNumKnobs]; }
    // The LED's light output, 0..1: brightness cubed, even steps to the eye
    float Led(uint32_t now_ms, const Engine& engine) const;

private:
    // A value set by a knob shared between layers. After a layer change the knob
    // moves the value from where it is, scaled so both reach the end stops together:
    // no jump, no dead zone.
    class Takeover {
    public:
        explicit Takeover(float value = 0.0f) : _value{ value }, _previous{ value } {}

        void Engage(float knob) { _previous = _origin = knob; }
        void Process(float knob);
        float Value() const { return _value; }

    private:
        static constexpr float kDeadband = 0.02f; // around the knob at Engage(): noise

        float _value;
        float _previous;
        float _origin = -1.0f; // knob at Engage() until it moves past the deadband
    };

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
