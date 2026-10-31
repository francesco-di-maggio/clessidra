#pragma once

#include <cstddef>

namespace clessidra
{
inline constexpr size_t kSampleRateHz = 48000;

// The buffer, mono float in SDRAM
inline constexpr size_t kBufferSeconds = 8;
inline constexpr size_t kBufferSamples = kBufferSeconds * kSampleRateHz;

} // namespace clessidra
