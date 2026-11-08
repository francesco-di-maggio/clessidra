#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
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
    static constexpr size_t kShakePad  = 0; // held: Spray at its top
    static constexpr size_t kMutePad   = 1; // held: the grains silent
    static constexpr size_t kHoldPad   = 2; // held: the playhead stops
    static constexpr size_t kNumPlaces = 7; // the selection in sevenths
    // P03-P09, the places, left to right on the board
    static constexpr std::array<size_t, kNumPlaces> kPlacePads
        = {3, 4, 8, 5, 9, 6, 7};
    static constexpr size_t kRecordPad = 10; // tap or hold
    static constexpr size_t kTurnPad   = 11; // the glass turned over

    // S37, Density: none at the centre, regular grains below, random above,
    // more of them further out. How many sound at once, exponential: grains
    // per second follow the size.
    static constexpr float kDensityDetent = 0.04f; // either side of the centre
    static constexpr float kMinOverlap    = 1.0f / 16.0f;
    static constexpr float kMaxOverlap    = 16.0f;

    // S31, Size: from kMinGrainMs to kMaxGrainMs, exponential
    static constexpr float kMinGrainMs = 5.0f;
    static constexpr float kMaxGrainMs = 2000.0f;

    // S09, Reverb: down off, centre short, up long. Core::Params.reverb.
    static constexpr float kReverbShort = 0.35f; // 1.7 s, a quarter wet
    static constexpr float kReverbLong  = 0.8f;  // 8 s, more than half wet

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

  private:
    // Pitch unison, speed 1 either way, size centre, Hann
    static constexpr size_t kNumDetents = 4;

    void Record(const Controls& c, Core& core, Core::Params& p);
    // P11 and the places. True while a place is held.
    bool Pads(const Controls& c, Core& core);
    void Pulse(const Controls& c, const std::array<bool, kNumDetents>& at);

    bool                         record_       = false;
    uint32_t                     rec_since_    = 0; // ms
    bool                         turn_touched_ = false;
    std::array<bool, kNumPlaces> places_touched_{};
    // LED. Times in ms, shown until they pass.
    uint32_t pulse_until_ = 0;
    // At each detent, and the knob when the pulse last started
    std::array<bool, kNumDetents>  at_detent_{};
    std::array<float, kNumDetents> detent_knob_{};
    bool                           detents_ready_ = false;
};

} // namespace clessidra
