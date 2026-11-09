#pragma once

#include <array>
#include <cstddef>

namespace clessidra
{
// A recording's onsets, its sharpest rises in level, found once per recording.
// From them, the slices of any loop: its start, then its strongest onsets in
// order. A loop with too few, as a sustained sound: the longest slices halved
// until there are enough.
class Onsets
{
  public:
    static constexpr size_t kNumSlices = 7;
    // Kept per recording, the strongest
    static constexpr size_t kMaxOnsets = 64;
    // Read in frames of this
    static constexpr float kFrameSeconds = 0.005f;

    // A recording to read, from its start: data stays valid until done
    void Begin(const float* data, size_t length, float sample_rate);
    // Reads on, up to frames of kFrameSeconds; true once the recording is
    // read. Steps keep each control pass short, the web's within a block.
    bool Step(size_t frames);
    // The whole recording at once
    void Find(const float* data, size_t length, float sample_rate)
    {
        Begin(data, length, sample_rate);
        Step(length);
    }
    // Where each slice starts, 0..1 of the loop from start, length long, both
    // 0..1 of the recording, round its end
    std::array<float, kNumSlices> Slices(float start, float length) const;

  private:
    static constexpr size_t kHistoryFrames = 6;     // a rise from 30 ms
    static constexpr float  kRise          = 4.0f;  // in power: 6 dB
    static constexpr float  kMinPower      = 1e-5f; // -50 dBFS
    static constexpr float  kMinGapSeconds = 0.05f;
    static constexpr float kPreRollSeconds = 0.005f; // a slice before its onset

    struct Onset
    {
        float at   = 0.0f; // 0..1 of the recording
        float rise = 0.0f; // 0: none
    };

    // Replaces the weakest kept, if weaker
    void Keep(const Onset& onset);

    std::array<Onset, kMaxOnsets> onsets_{};
    // Reading
    const float*                      data_   = nullptr;
    size_t                            length_ = 0;
    size_t                            frame_  = 1; // samples
    size_t                            frames_ = 0;
    size_t                            gap_    = 0; // frames
    float                             pre_    = 0.0f;
    size_t                            next_   = 0; // frame
    std::array<float, kHistoryFrames> history_{};
    float                             history_sum_ = 0.0f;
    size_t                            frame_at_    = 0; // of current_
    Onset                             current_;
};

} // namespace clessidra
