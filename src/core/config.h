#pragma once

#include <cstddef>

namespace clessidra
{
inline constexpr size_t kSampleRateHz = 48000;

// The longest recording, mono float in SDRAM, in each of two buffers. The ring
// holds it, its pre-roll and the time to copy it.
inline constexpr size_t kBufferSeconds = 30;
inline constexpr size_t kBufferSamples = kBufferSeconds * kSampleRateHz;
inline constexpr size_t kRingSamples   = kBufferSamples + kSampleRateHz;

} // namespace clessidra
