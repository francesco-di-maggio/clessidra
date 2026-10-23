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
static constexpr size_t   kRecordSeconds = 8;
static constexpr size_t   kRecordSamples = kRecordSeconds * kSampleRateHz;
static constexpr size_t   kRingSamples   = (kRecordSeconds + 1) * kSampleRateHz;

// MAIN LOOP ///////////////////////////////////////////////////////////////
static constexpr uint32_t kLoopMs = 3;   // pad scan interval
static constexpr uint32_t kLogMs  = 250; // serial log interval (LOG=1)
