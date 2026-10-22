#pragma once

#include <cstddef>
#include <cstdint>
#include "daisy_seed.h"

// Device name shown on USB
#define CLESSIDRA_MANUFACTURER "Synthux"
#define CLESSIDRA_PRODUCT      "Clessidra"

// AUDIO ///////////////////////////////////////////////////////////////////
static constexpr auto     kSampleRate   = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
static constexpr size_t   kSampleRateHz = 48000; // matches kSampleRate
static constexpr size_t   kBlockSize    = 48;    // 1 ms at 48 kHz

// BUFFER //////////////////////////////////////////////////////////////////
// Mono float in SDRAM: two recordings and an input ring 1 s longer, which holds the
// tap, pre-roll and copy margin (Engine constants). About 80 s fit in 64 MB.
static constexpr size_t   kRecordSeconds = 10;
static constexpr size_t   kRecordSamples = kRecordSeconds * kSampleRateHz;
static constexpr size_t   kRingSamples   = (kRecordSeconds + 1) * kSampleRateHz;

// PADS ////////////////////////////////////////////////////////////////////
static constexpr size_t   kRecordPad = 10; // P10, tap or hold to record
static constexpr size_t   kShiftPad  = 11; // P11, hold for the second knob layer
static constexpr size_t   kStopPad   = 1;  // P01, hold to stop the flow
// The engine's places, first to last in the selection: pads left to right on the
// faceplate, top to bottom within a column. Pressure sets each place's flow.
static constexpr size_t   kPlacePads[] = { 3, 0, 4, 8, 5, 2, 6, 9, 7 };

// MAIN LOOP ///////////////////////////////////////////////////////////////
static constexpr uint32_t kLoopMs = 3;   // pad scan interval
static constexpr uint32_t kLogMs  = 250; // serial log interval (LOG=1)

// LED /////////////////////////////////////////////////////////////////////
static constexpr float    kGlow          = 0.6f; // brightness while grains play, below the pulse
static constexpr uint32_t kRecordFlashMs = 125;  // on and off while recording: 4 Hz
static constexpr uint32_t kPulseMs       = 150;  // on when pitch, speed, grain size or tone reaches its centre
static constexpr float    kShowStep      = 0.01f; // second-layer change that counts as turning
