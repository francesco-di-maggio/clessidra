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
    static constexpr size_t kFlipPad    = 11; // the glass turned over
    // Notes, lowest to highest: pads left to right on the faceplate, top to
    // bottom within a column
    static constexpr size_t kNotePads[] = {3, 4, 8, 5, 6, 9, 7};
    static_assert(std::size(kNotePads) == Core::kNumNotes);
    // S09, the notes' scale, semitones above Pitch: up major, centre minor,
    // down major pentatonic
    static constexpr float kMajor[]      = {0, 2, 4, 5, 7, 9, 11};
    static constexpr float kMinor[]      = {0, 2, 3, 5, 7, 8, 10};
    static constexpr float kPentatonic[] = {0, 2, 4, 7, 9, 12, 14};

    // S37, Density: none at the bottom, then grains per second, exponential.
    // A size and the reverb follow it.
    static constexpr float kDensityOff       = 0.02f;  // none below this
    static constexpr float kDensityMin       = 0.3f;   // grains per second
    static constexpr float kDensityGrainMsLo = 30.0f;  // sparse
    static constexpr float kDensityGrainMsHi = 300.0f; // dense
    static constexpr float kReverbSparse     = 0.7f;   // a lone grain
    static constexpr float kReverbDense      = 0.15f;
    // From kCloudFrom, Spray at least kCloudSpray at the top: grains that all
    // read the same spot sum to the recording, not a cloud
    static constexpr float kCloudFrom  = 0.6f;
    static constexpr float kCloudSpray = 0.3f;

    // S31, Size: two octaves either way of Density's, within these
    static constexpr float kMinGrainMs = 10.0f;
    static constexpr float kMaxGrainMs = 4000.0f;
    // S32, Spray: random pitch over its top quarter
    static constexpr float kPitchSprayFrom = 0.75f;
    // pad pressure above a light touch (Pads: measured 0.3-0.5)
    static constexpr float kPressFrom = 0.5f;
    // A note's density: the fader's, at least this, raised by pressure
    static constexpr float kNoteDensity = 0.5f;
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
    void Pads(const Controls& c, Core& core);
    void Notes(const Controls& c, float density, Core::Params& p);
    void Pulse(const Controls& c, const std::array<bool, kNumDetents>& at);

    bool     record_          = false;
    uint32_t rec_since_       = 0; // ms
    bool     seed_touched_    = false;
    bool     flip_touched_    = false;
    bool     restart_touched_ = false;
    // LED. Times in ms, shown until they pass.
    uint32_t pulse_until_ = 0;
    // At each detent, and the knob when the pulse last started
    std::array<bool, kNumDetents>  at_detent_{};
    std::array<float, kNumDetents> detent_knob_{};
    bool                           detents_ready_ = false;
};

} // namespace clessidra
