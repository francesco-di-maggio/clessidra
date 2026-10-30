#pragma once

#include <array>
#include <cstddef>

namespace clessidra
{
// Divides a recording at its sharpest rises in level: the slices the pads play.
// No onsets to find, as in a sustained sound: the longest slices are halved
// until there are enough.
inline constexpr size_t kNumSlices = 7;

// Slice edges, 0..1 of the recording, first to last. Not for the audio
// callback: reads the whole recording.
std::array<float, kNumSlices + 1>
FindSlices(const float* data, size_t length, float sample_rate);

} // namespace clessidra
