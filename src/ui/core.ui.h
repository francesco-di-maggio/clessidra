#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include "core/config.h"
#include "core/core.h"
#include "core/onsets.h"
#include "hw/controls.h"

namespace clessidra
{
// Maps the Simple Touch's controls to the core's parameters and recording, and
// drives the LED. One layer: every control does one thing, where it is. No
// hardware code: the board and the host tests run the same mapping.
class CoreUI
{
  public:
    // Pads
    static constexpr size_t kMutePad    = 0;  // held: the grains silent
    static constexpr size_t kDropPad    = 1;  // a touch: one grain
    static constexpr size_t kRestartPad = 2;  // the sand back to the top
    static constexpr size_t kRecordPad  = 10; // tap or hold
    static constexpr size_t kShakePad   = 11; // held: the sand shaken
    // Slices, first to last in the buffer: pads left to right on the
    // faceplate, top to bottom within a column
    static constexpr size_t kSlicePads[] = {3, 4, 8, 5, 6, 9, 7};
    static_assert(std::size(kSlicePads) == kNumSlices);
    static_assert(kNumSlices == Core::kNumTriggers);

    // S37, Flow: the neck, from closed to single drops, a stream and a cloud.
    // Density, size, window and reverb follow the one gesture.
    static constexpr float kFlowOff       = 0.02f;  // closed below this
    static constexpr float kDensityMin    = 0.3f;   // grains per second
    static constexpr float kFlowGrainMsLo = 30.0f;  // drops
    static constexpr float kFlowGrainMsHi = 300.0f; // cloud
    static constexpr float kReverbDrops   = 0.7f;   // alone in a big glass
    static constexpr float kReverbCloud   = 0.15f;
    // window: percussive up to kShapeFrom, Hann from kShapeTo
    static constexpr float kShapeFrom = 0.25f;
    static constexpr float kShapeTo   = 0.6f;
    // From kCloudFrom, Scatter at least kCloudScatter at the top: grains that
    // all read the same spot sum to the recording, not a cloud
    static constexpr float kCloudFrom    = 0.6f;
    static constexpr float kCloudScatter = 0.3f;

    // S31, Size: two octaves either way of Flow's, within these
    static constexpr float kMinGrainMs = 10.0f;
    static constexpr float kMaxGrainMs = 4000.0f;
    // S32, Scatter: random pitch over its top quarter
    static constexpr float kPitchScatterFrom = 0.75f;
    // P11, Shake: Scatter towards the top, part way at a light touch
    static constexpr float kShakeFrom = 0.3f;
    // pad pressure above a light touch (Pads: measured 0.3-0.5)
    static constexpr float kPressFrom = 0.5f;
    // A slice's flow: the fader's, at least this, opened by pressure
    static constexpr float kSliceFlow = 0.5f;
    // P10 held past a tap: the input heard with the grains, equal power
    static constexpr float kMonitorMix = 0.5f;

    // LED
    // brightness while grains play, below the pulse
    static constexpr float kGlow = 0.6f;
    // of the glow with the sand run out: the glow follows the sand left
    static constexpr float kSandFloor = 0.25f;
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
    // For displays: slice edges, of the recording
    const std::array<float, kNumSlices + 1>& Slices() const { return slices_; }

  private:
    // Pitch unison, speed 1, size centre, tone off
    static constexpr size_t kNumDetents = 4;

    void EqualSlices();
    void Record(const Controls& c, Core& core, Core::Params& p);
    void Pads(const Controls& c, Core& core);
    void Triggers(const Controls& c, float flow, Core::Params& p);
    void Pulse(const Controls& c, const std::array<bool, kNumDetents>& at);

    std::array<bool, kNumSlices>          slice_touched_{};
    std::array<Core::Trigger, kNumSlices> triggers_{};
    std::array<float, kNumSlices + 1>     slices_{}; // edges, of the recording
    uint32_t                              recordings_      = 0;
    bool                                  record_          = false;
    uint32_t                              rec_since_       = 0; // ms
    bool                                  drop_touched_    = false;
    bool                                  restart_touched_ = false;
    // LED. Times in ms, shown until they pass.
    uint32_t pulse_until_ = 0;
    // At each detent, and the knob when the pulse last started
    std::array<bool, kNumDetents>  at_detent_{};
    std::array<float, kNumDetents> detent_knob_{};
    bool                           detents_ready_ = false;
};

} // namespace clessidra
