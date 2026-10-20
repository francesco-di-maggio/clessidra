#pragma once

#include <cstddef>
#include <cstdint>
#include "daisy_seed.h"

// Device name shown on USB
#define CLESSIDRA_MANUFACTURER "Synthux"
#define CLESSIDRA_PRODUCT      "Clessidra"

// AUDIO ///////////////////////////////////////////////////////////////////
static constexpr auto     kSampleRate = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
static constexpr size_t   kBlockSize  = 48; // 1 ms at 48 kHz

// MAIN LOOP ///////////////////////////////////////////////////////////////
static constexpr uint32_t kLoopMs = 3;   // pad scan interval
static constexpr uint32_t kLogMs  = 250; // serial log interval (LOG=1)
