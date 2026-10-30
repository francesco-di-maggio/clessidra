#include "onsets.h"

#include <algorithm>

namespace clessidra
{
namespace
{
    constexpr float  kFrameSeconds   = 0.005f;
    constexpr size_t kHistoryFrames  = 6;     // what a frame rises from: 30 ms
    constexpr float  kRise           = 4.0f;  // power: 6 dB
    constexpr float  kMinPower       = 1e-5f; // -50 dBFS
    constexpr float  kMinGapSeconds  = 0.05f;
    constexpr float  kPreRollSeconds = 0.005f; // an edge, before its onset

    struct Onset
    {
        size_t frame = 0;
        float  rise  = 0.0f;
    };
} // namespace

std::array<float, kNumSlices + 1>
FindSlices(const float* data, size_t length, float sample_rate)
{
    std::array<float, kNumSlices + 1> edges{};
    edges[kNumSlices] = 1.0f;
    const size_t frame
        = std::max(size_t(kFrameSeconds * sample_rate), size_t(1));
    const size_t frames = length / frame;
    const size_t gap    = size_t(kMinGapSeconds / kFrameSeconds);

    // The strongest, weakest of them replaced first
    std::array<Onset, kNumSlices - 1> found{};
    std::array<float, kHistoryFrames> history{};
    float                             history_sum = 0.0f;
    Onset                             current;
    auto                              keep = [&] {
        if(current.rise == 0.0f)
            return;
        auto weakest = std::min_element(
            found.begin(), found.end(), [](const Onset& a, const Onset& b) {
                return a.rise < b.rise;
            });
        if(current.rise > weakest->rise)
            *weakest = current;
    };
    for(size_t f = 0; f < frames; f++)
    {
        float power = 0.0f;
        for(size_t i = f * frame; i < (f + 1) * frame; i++)
            power += data[i] * data[i];
        power /= float(frame);

        // Not the start: it is an edge already
        const float before = history_sum / float(kHistoryFrames);
        if(f >= gap && power > kMinPower && power > kRise * before)
        {
            const float rise = power / std::max(before, kMinPower);
            // Within the gap, one onset: the strongest rise
            if(current.rise > 0.0f && f >= current.frame + gap)
            {
                keep();
                current = Onset{};
            }
            if(rise > current.rise)
                current = {f, rise};
        }
        const size_t slot = f % kHistoryFrames;
        history_sum += power - history[slot];
        history[slot] = power;
    }
    keep();

    // Unused entries, of rise 0, sort first
    std::sort(found.begin(), found.end(), [](const Onset& a, const Onset& b) {
        return a.frame < b.frame;
    });
    size_t edge = 1;
    for(const Onset& onset : found)
    {
        if(onset.rise == 0.0f)
            continue;
        const float seconds
            = float(onset.frame) * kFrameSeconds - kPreRollSeconds;
        edges[edge++] = std::max(seconds * sample_rate / float(length), 0.0f);
    }

    // Too few: halve the longest until there are enough
    size_t last = edge; // edges[last] is the end from here
    edges[last] = 1.0f;
    while(last < kNumSlices)
    {
        size_t longest = 0;
        for(size_t i = 1; i < last; i++)
        {
            if(edges[i + 1] - edges[i] > edges[longest + 1] - edges[longest])
                longest = i;
        }
        for(size_t i = last + 1; i > longest + 1; i--)
            edges[i] = edges[i - 1];
        edges[longest + 1] = 0.5f * (edges[longest] + edges[longest + 2]);
        last++;
    }
    return edges;
}

} // namespace clessidra
