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
// P00, P02-P09: pressure raises flow
static constexpr size_t   kPourPads[] = { 0, 2, 3, 4, 5, 6, 7, 8, 9 };

// MAIN LOOP ///////////////////////////////////////////////////////////////
static constexpr uint32_t kLoopMs = 3;   // pad scan interval
static constexpr uint32_t kLogMs  = 250; // serial log interval (LOG=1)
