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
// Mono float in SDRAM: 30 s = 5.5 MB of 64 MB.
static constexpr size_t   kBufferSeconds = 30;
static constexpr size_t   kBufferSamples = kBufferSeconds * kSampleRateHz;

// PADS ////////////////////////////////////////////////////////////////////
static constexpr size_t   kRecordPad = 10; // P10, hold to record
static constexpr size_t   kShiftPad  = 11; // P11, hold for the second knob layer
static constexpr size_t   kStopPad   = 1;  // P01, hold to stop the flow
// The engine's places, first to last in the selection: pads left to right on the
// faceplate, top to bottom within a column. Pressure sets each place's flow.
static constexpr size_t   kPlacePads[] = { 3, 0, 4, 8, 5, 2, 6, 9, 7 };

// MAIN LOOP ///////////////////////////////////////////////////////////////
static constexpr uint32_t kLoopMs = 3;   // pad scan interval
static constexpr uint32_t kLogMs  = 250; // serial log interval (LOG=1)

// LED /////////////////////////////////////////////////////////////////////
static constexpr uint32_t kBlinkMs       = 10;  // on per grain start
static constexpr uint32_t kRecordFlashMs = 125; // on and off while recording: 4 Hz
