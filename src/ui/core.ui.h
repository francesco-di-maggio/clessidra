#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include "core/config.h"
#include "core/core.h"
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
    static constexpr size_t kSeedPad    = 0;  // a touch: one grain
    static constexpr size_t kMutePad    = 1;  // held: the grains silent
    static constexpr size_t kRestartPad = 2;  // the playhead back to the start
    static constexpr size_t kRecordPad  = 10; // tap or hold
    // Gestures, held: pads left to right on the faceplate, then P11
    enum Gesture : size_t
    {
        Clog,
        Burst,
        Fine,
        Shake,
        Drag,
        Rush,
        Far,
        Reverse,
        kNumGestures
    };
    static constexpr size_t kGesturePads[] = {3, 4, 8, 5, 6, 9, 7, 11};
    static_assert(std::size(kGesturePads) == kNumGestures);
    // a light touch half way, firm all the way; in fast, settling back
    static constexpr float kGestureLight     = 0.5f;
    static constexpr float kGestureAttackMs  = 30.0f;
    static constexpr float kGestureReleaseMs = 300.0f;
    static constexpr float kFineMs           = 8.0f; // Fine's grain size
    // speed below this, either way: held
    static constexpr float kStopped = 0.001f;

    // S37, Density: none at the bottom, then grains per second, exponential.
    // The reverb follows it.
    static constexpr float kDensityOff   = 0.02f; // none below this
    static constexpr float kDensityMin   = 0.3f;  // grains per second
    static constexpr float kReverbSparse = 0.7f;  // a lone grain
    static constexpr float kReverbDense  = 0.15f;

    // S31, Size: from kMinGrainMs to kMaxGrainMs, exponential
    static constexpr float kMinGrainMs = 5.0f;
    static constexpr float kMaxGrainMs = 2000.0f;
    // S32, Spray: random pitch over its top quarter
    static constexpr float kPitchSprayFrom = 0.75f;
    // pad pressure above a light touch (Pads: measured 0.3-0.5)
    static constexpr float kPressFrom = 0.5f;
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
    // For displays: how far a gesture bends the glass now, 0..1
    float Depth(Gesture g) const { return depth_[g]; }

  private:
    // Pitch unison, speed 1 either way, size centre, Hann
    static constexpr size_t kNumDetents = 4;

    void Record(const Controls& c, Core& core, Core::Params& p);
    void Pads(const Controls& c, Core& core);
    void Gestures(const Controls& c, float spray, Core::Params& p);
    void Pulse(const Controls& c, const std::array<bool, kNumDetents>& at);

    bool     record_          = false;
    uint32_t rec_since_       = 0; // ms
    bool     seed_touched_    = false;
    bool     restart_touched_ = false;
    // Each gesture's depth, 0..1, and when it was last followed, ms
    std::array<float, kNumGestures> depth_{};
    uint32_t                        gesture_ms_ = 0;
    // LED. Times in ms, shown until they pass.
    uint32_t pulse_until_ = 0;
    // At each detent, and the knob when the pulse last started
    std::array<bool, kNumDetents>  at_detent_{};
    std::array<float, kNumDetents> detent_knob_{};
    bool                           detents_ready_ = false;
};

} // namespace clessidra
