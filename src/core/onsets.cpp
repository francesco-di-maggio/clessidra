#include "onsets.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
void Onsets::Find(const float* data, size_t length, float sample_rate)
{
    onsets_.fill(Onset{});
    if(length == 0)
        return;
    const size_t frame
        = std::max(size_t(kFrameSeconds * sample_rate), size_t(1));
    const size_t frames = length / frame;
    const size_t gap    = size_t(kMinGapSeconds / kFrameSeconds);
    const float  pre    = kPreRollSeconds * sample_rate / float(length);

    std::array<float, kHistoryFrames> history{};
    float                             history_sum = 0.0f;
    size_t                            frame_at    = 0; // of current
    Onset                             current;
    for(size_t f = 0; f < frames; f++)
    {
        float power = 0.0f;
        for(size_t i = f * frame; i < (f + 1) * frame; i++)
            power += data[i] * data[i];
        power /= float(frame);

        // A rise over the frames before it. Within the gap, one onset: the
        // strongest.
        const float before = history_sum / float(kHistoryFrames);
        if(f >= kHistoryFrames && power > kMinPower && power > kRise * before)
        {
            const float rise = power / std::max(before, kMinPower);
            if(current.rise > 0.0f && f >= frame_at + gap)
            {
                Keep(current);
                current = Onset{};
            }
            if(rise > current.rise)
            {
                current
                    = {std::max(float(f * frame) / float(length) - pre, 0.0f),
                       rise};
                frame_at = f;
            }
        }
        const size_t slot = f % kHistoryFrames;
        history_sum += power - history[slot];
        history[slot] = power;
    }
    Keep(current);
}

void Onsets::Keep(const Onset& onset)
{
    if(onset.rise == 0.0f)
        return;
    auto weakest = std::min_element(
        onsets_.begin(), onsets_.end(), [](const Onset& a, const Onset& b) {
            return a.rise < b.rise;
        });
    if(onset.rise > weakest->rise)
        *weakest = onset;
}

std::array<float, Onsets::kNumSlices> Onsets::Slices(float start,
                                                     float length) const
{
    // The loop's onsets, strongest first, as phases of the loop; none at its
    // very start, where the first slice is
    std::array<Onset, kMaxOnsets> inside{};
    size_t                        count = 0;
    for(const Onset& o : onsets_)
    {
        if(o.rise == 0.0f || length <= 0.0f)
            continue;
        float from = o.at - start;
        from -= std::floor(from);
        const float phase = from / length;
        if(phase > 0.01f && phase < 1.0f)
            inside[count++] = {phase, o.rise};
    }
    std::sort(inside.begin(),
              inside.begin() + count,
              [](const Onset& a, const Onset& b) { return a.rise > b.rise; });
    count = std::min(count, kNumSlices - 1);

    // Edges in order, the loop's end last
    std::array<float, kNumSlices + 1> edges{};
    for(size_t i = 0; i < count; i++)
        edges[i + 1] = inside[i].at;
    std::sort(edges.begin() + 1, edges.begin() + 1 + count);
    size_t last = count + 1;
    edges[last] = 1.0f;
    while(last < kNumSlices)
    {
        size_t longest = 0;
        for(size_t i = 1; i < last; i++)
            if(edges[i + 1] - edges[i] > edges[longest + 1] - edges[longest])
                longest = i;
        for(size_t i = last + 1; i > longest + 1; i--)
            edges[i] = edges[i - 1];
        edges[longest + 1] = 0.5f * (edges[longest] + edges[longest + 2]);
        last++;
    }

    std::array<float, kNumSlices> slices{};
    std::copy(edges.begin(), edges.begin() + kNumSlices, slices.begin());
    return slices;
}

} // namespace clessidra
