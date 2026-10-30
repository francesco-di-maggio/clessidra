#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include "common/takeover.h"
#include "core/config.h"
#include "core/core.h"
#include "core/onsets.h"
#include "hw/controls.h"

namespace clessidra
{
// Maps the Simple Touch's controls to the core's parameters, and drives the
// LED. Holding P11 switches the knobs to the effects. No hardware code: the
// board and the host tests run the same mapping.
class CoreUI
{
  public:
    // Pads
    static constexpr size_t kRecordPad  = 10; // P10, tap or hold to record
    static constexpr size_t kEffectsPad = 11; // P11, hold for the effects
    // Slices, first to last in the recording: pads left to right on the
    // faceplate, top to bottom within a column
    static constexpr size_t kSlicePads[] = {3, 4, 8, 5, 6, 9, 7};
    static_assert(std::size(kSlicePads) == kNumSlices);

    // P11 held
    static constexpr size_t kInputKnob    = Controls::S36;
    static constexpr size_t kOutputKnob   = Controls::S37;
    static constexpr size_t kMixKnob      = Controls::S35;
    static constexpr size_t kReverbKnob   = Controls::S32;
    static constexpr size_t kResampleKnob = Controls::S33;
    static constexpr size_t kToneKnob     = Controls::S34;

    // of the recording: the shortest selection
    static constexpr float kMinLength  = 0.01f;
    static constexpr float kMaxGrainMs = 4000.0f;
    // S32 at the top: grains this far either way from the playhead
    static constexpr float kMaxScatterSeconds = 2.0f;
    // S36: distance from the centre, of half its travel, that stops
    static constexpr float kStopWidth = 0.04f;
    static constexpr float kMaxInput  = 4.0f; // input gain at the top: +12 dB
    // pad pressure above a light touch (Pads: measured 0.3-0.5)
    static constexpr float kPourFrom = 0.5f;

    // LED
    // brightness while grains play, below the pulse
    static constexpr float kGlow = 0.6f;
    // on and off while P10 is touched: 4 Hz
    static constexpr uint32_t kRecordFlashMs = 125;
    // on when a control reaches its centre and while turned there
    static constexpr uint32_t kPulseMs = 300;
    // turn within a centre that renews the pulse
    static constexpr float kPulseStep = 0.01f;
    // effects change that counts as turning
    static constexpr float kShowStep = 0.01f;

    CoreUI();
    CoreUI(const CoreUI&) = delete;
    CoreUI& operator=(const CoreUI&) = delete;

    // Every control pass. Sets the core's parameters and recording.
    void Process(const Controls& c, Core& core);
    // A knob's effects value
    float Effect(size_t knob) const { return effects_[knob]; }
    // The LED's light output, 0..1: brightness cubed, even steps to the eye
    float Led(uint32_t now_ms, const Core& core) const;

  private:
    // A value per knob
    struct Layer
    {
        std::array<Takeover, Controls::kNumKnobs> values;

        float operator[](size_t knob) const { return values[knob].Value(); }
        void  Engage(const Controls& c);
        void  Process(const Controls& c);
    };

    // A layer's values at their last move, and the knob last turned: what the
    // LED shows
    struct Shown
    {
        std::array<float, Controls::kNumKnobs> values{};
        size_t                                 knob = 0;

        void Update(const Layer& layer);
    };

    Layer                        main_;
    Layer                        effects_;
    Layer*                       layer_      = &main_;
    bool                         started_    = false;
    bool                         effects_on_ = false;
    std::array<bool, kNumSlices> slice_touched_{};
    size_t                       slice_ = kNumSlices; // the whole selection
    std::array<float, kNumSlices + 1> slices_{};      // edges, of the recording
    uint32_t                          recordings_seen_ = 0;
    // LED. Times in ms, shown until they pass.
    uint32_t pulse_until_  = 0;
    uint32_t record_since_ = 0; // P10 touched
    bool     recording_    = false;
    Shown    shown_; // at power-up: dry/wet
    // Pitch unison, stop, speed 1, grain size 100 ms, shape Hann, tone off,
    // input unity: at the centre, and the knob when the pulse last started
    static constexpr size_t        kNumDetents = 7;
    std::array<bool, kNumDetents>  at_detent_{};
    std::array<float, kNumDetents> detent_knob_{};
    bool                           detents_ready_ = false;
};

} // namespace clessidra
