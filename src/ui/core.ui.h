#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "core/config.h"
#include "core/core.h"
#include "core/onsets.h"
#include "hw/controls.h"
#include "takeover.h"

namespace clessidra
{
// Maps the Simple Touch's controls to the core's parameters and recording, and
// drives the LED. Two layers: each control's own, and P11 held (Shift) for the
// knobs' twins. A knob moves a value from where it is after a layer change,
// scaled so both reach the end stops together. No hardware code: the board and
// the host tests run the same mapping.
class CoreUI
{
  public:
    // Pads
    static constexpr size_t kStopPad
        = 0; // a touch: stop as a tape machine, or start
    static constexpr size_t kResetPad = 1; // with Shift: the twins reset
    static constexpr size_t kCuePad
        = 2; // cue: the playhead back to the loop's start
    static constexpr size_t kRecordPad = 10; // tap or hold. With Shift: listen
    static constexpr size_t kShiftPad  = 11; // held: the second layer
    static constexpr size_t kNumSlices = Onsets::kNumSlices;
    // P03-P09, the slices, left to right on the board
    static constexpr std::array<size_t, kNumSlices> kSlicePads
        = {3, 4, 8, 5, 9, 6, 7};

    // S37, Density: none at the centre, regular grains below, random above,
    // more of them further out. How many sound at once, exponential: grains
    // per second follow the size.
    static constexpr float kDensityDetent = 0.04f; // either side of the centre
    static constexpr float kMinOverlap    = 1.0f / 16.0f;
    static constexpr float kMaxOverlap    = 16.0f;

    // S31, Size: from kMinGrainMs to kMaxGrainMs, exponential
    static constexpr float kMinGrainMs = 5.0f;
    static constexpr float kMaxGrainMs = 2000.0f;

    // The second layer's values at power-up and after a reset: no randomness,
    // the filter off, a short reverb (1.7 s, a quarter wet), all wet
    static constexpr float kRandomPitch = 0.0f;
    static constexpr float kRandomSize  = 0.0f;
    static constexpr float kFilter      = 0.5f;
    static constexpr float kReverb      = 0.35f;
    static constexpr float kDryWet      = 1.0f;

    // S36 frozen: moves under this are noise, not scrubbing
    static constexpr float kScrubStep = 0.002f;

    // P10 held past a tap, or listening: the input heard with the grains,
    // equal power
    static constexpr float kMonitorMix = 0.5f;

    // LED
    // brightness while grains play, below the pulse
    static constexpr float kGlow = 0.6f;
    // of the glow with the sand run out: the glow follows the sand left
    static constexpr float kSandFloor = 0.25f;
    // Shift held: at least this, else the value last turned
    static constexpr float kShiftFloor = 0.15f;
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
    // Where each slice starts in the loop, 0..1, at the last pass
    const std::array<float, kNumSlices>& SliceStarts() const
    {
        return slice_starts_;
    }

  private:
    // Pitch unison, speed 1, size centre, Hann, filter off
    static constexpr size_t kNumDetents = 5;
    // Knobs whose own value means most at the centre: Size, Pitch, Shape,
    // Density
    static constexpr std::array<bool, Controls::kNumKnobs> kCentred
        = {false, true, false, true, true, false, false, true};
    // Knobs with a twin on the second layer
    static constexpr std::array<bool, Controls::kNumKnobs> kTwinned
        = {false, true, false, true, true, false, true, true};

    // The knobs' values on both layers. Frozen, S36 scrubs instead.
    void Knobs(const Controls& c, bool frozen);
    void Record(const Controls& c, Core& core, Core::Params& p);
    // The slices of the loop, at the recording's onsets. True while one is
    // held.
    bool Slices(const Controls& c, Core& core, float start, float length);
    void Scrub(float fader, bool frozen, Core& core);
    void Reset(const Controls& c);
    // P00 Stop, P02 Cue
    void Transport(const Controls& c, Core& core, Core::Params& p);
    void Pulse(const Controls& c, const std::array<bool, kNumDetents>& at);

    std::array<Takeover, Controls::kNumKnobs> own_{};
    std::array<Takeover, Controls::kNumKnobs> twin_{};
    bool                                      ready_  = false;
    bool                                      shift_  = false;
    bool                                      frozen_ = false;
    // S36 when it last scrubbed
    float scrub_from_ = 0.0f;
    // P10: touched, recording, or listening with Shift
    bool                          record_touched_ = false;
    bool                          listening_      = false;
    bool                          record_         = false;
    uint32_t                      rec_since_      = 0; // ms
    bool                          reset_touched_  = false;
    bool                          stop_touched_   = false;
    bool                          stop_           = false;
    bool                          cue_touched_    = false;
    std::array<bool, kNumSlices>  slices_touched_{};
    Onsets                        onsets_;
    uint32_t                      analysed_ = 0; // Core::Recordings() then
    std::array<float, kNumSlices> slice_starts_{};
    // LED. Times in ms, shown until they pass.
    uint32_t pulse_until_ = 0;
    float    shown_       = 0.0f; // the twin last turned, Shift held
    // At each detent, and the value when the pulse last started
    std::array<bool, kNumDetents>  at_detent_{};
    std::array<float, kNumDetents> detent_value_{};
    bool                           detents_ready_ = false;
};

} // namespace clessidra
