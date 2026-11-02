#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include "core/config.h"
#include "core/core.h"
#include "core/onsets.h"
#include "hw/controls.h"
#include "layers.h"

namespace clessidra
{
// Maps the Simple Touch's controls to the core's parameters and recording, and
// drives the LED. P11 held: each grain parameter's randomness, and reverb. No
// hardware code: the board and the host tests run the same mapping.
class CoreUI
{
  public:
    // Pads
    static constexpr size_t kRecordPad  = 10;
    static constexpr size_t kLayerPad   = 11; // held: the second layer
    static constexpr size_t kRestartPad = 2;
    // Slices, first to last in the buffer: pads left to right on the
    // faceplate, top to bottom within a column
    static constexpr size_t kSlicePads[] = {3, 4, 8, 5, 6, 9, 7};
    static_assert(std::size(kSlicePads) == kNumSlices);
    static_assert(kNumSlices == Core::kNumTriggers);

    // of the recording: the shortest selection
    static constexpr float kMinLength  = 0.01f;
    static constexpr float kMinGrainMs = 10.0f;
    static constexpr float kMaxGrainMs = 4000.0f;
    // Second layer at power-up: randomness off at the centre, some reverb
    static constexpr float kRandomStart = 0.5f;
    static constexpr float kReverbStart = 0.25f;
    // S37: distance from the centre that is off, as Beads
    static constexpr float kDensityStop = 0.02f;
    // S36: distance from the centre, of half its travel, that stops
    static constexpr float kStopWidth = 0.04f;
    // pad pressure above a light touch (Pads: measured 0.3-0.5)
    static constexpr float kPressFrom = 0.5f;
    // A trigger's density: Density's travel at a light touch, the top when firm
    static constexpr float kTriggerFrom = 0.5f;

    // LED
    // brightness while grains play, below the pulse
    static constexpr float kGlow = 0.6f;
    // on when a control reaches its centre and while turned there
    static constexpr uint32_t kPulseMs = 300;
    // turn within a centre that renews the pulse
    static constexpr float kPulseStep = 0.01f;
    // on and off while recording: 4 Hz
    static constexpr uint32_t kFlashMs = 125;

    CoreUI();
    CoreUI(const CoreUI&) = delete;
    CoreUI& operator=(const CoreUI&) = delete;

    // Every control pass. Sets the core's parameters and recording.
    void Process(const Controls& c, Core& core);
    // The LED's light output, 0..1: brightness cubed, even steps to the eye
    float Led(uint32_t now_ms, const Core& core) const;
    // For UIs with one control per function: a knob's second value directly
    void SetSecond(Controls::Knob knob, float value)
    {
        layers_.SetSecond(knob, value);
    }
    // For displays: slice edges, of the recording
    const std::array<float, kNumSlices + 1>& Slices() const { return slices_; }

  private:
    void EqualSlices();

    std::array<bool, kNumSlices>          slice_touched_{};
    bool                                  restart_touched_ = false;
    std::array<Core::Trigger, kNumSlices> triggers_{};
    std::array<float, kNumSlices + 1>     slices_{}; // edges, of the recording
    uint32_t                              recordings_ = 0;
    bool                                  record_     = false;
    bool                                  layer_      = false;
    Layers                                layers_;
    // LED. Times in ms, shown until they pass.
    uint32_t pulse_until_ = 0;
    // Pitch unison, speed 1, grain size centre, density off: there, and the
    // knob when the pulse last started
    static constexpr size_t        kNumDetents = 4;
    std::array<bool, kNumDetents>  at_detent_{};
    std::array<float, kNumDetents> detent_knob_{};
    bool                           detents_ready_ = false;
};

} // namespace clessidra
