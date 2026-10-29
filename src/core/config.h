#pragma once

#include <cstddef>

namespace clessidra {

static constexpr size_t kSampleRateHz = 48000;

// Mono float in SDRAM: two recordings and an input ring 1 s longer, which holds the
// tap, pre-roll and copy margin (Core constants). About 80 s fit in 64 MB.
static constexpr size_t kRecordSeconds = 8;
static constexpr size_t kRecordSamples = kRecordSeconds * kSampleRateHz;
static constexpr size_t kRingSamples   = (kRecordSeconds + 1) * kSampleRateHz;

} // namespace clessidra
