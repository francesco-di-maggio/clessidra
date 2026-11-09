#include "onsets.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
void Onsets::Begin(const float* data, size_t length, float sample_rate)
{
    onsets_.fill(Onset{});
    data_   = data;
    length_ = length;
    frame_  = std::max(size_t(kFrameSeconds * sample_rate), size_t(1));
    frames_ = length / frame_;
    gap_    = size_t(kMinGapSeconds / kFrameSeconds);
    pre_    = length > 0 ? kPreRollSeconds * sample_rate / float(length) : 0.0f;
    next_   = 0;
    history_     = {};
    history_sum_ = 0.0f;
    frame_at_    = 0;
    current_     = Onset{};
}

bool Onsets::Step(size_t frames)
{
    if(next_ >= frames_)
        return true;
    const size_t last = std::min(frames_, next_ + frames);
    for(size_t f = next_; f < last; f++)
    {
        float power = 0.0f;
        for(size_t i = f * frame_; i < (f + 1) * frame_; i++)
            power += data_[i] * data_[i];
        power /= float(frame_);

        // A rise over the frames before it. Within the gap, one onset: the
        // strongest.
        const float before = history_sum_ / float(kHistoryFrames);
        if(f >= kHistoryFrames && power > kMinPower && power > kRise * before)
        {
            const float rise = power / std::max(before, kMinPower);
            if(current_.rise > 0.0f && f >= frame_at_ + gap_)
            {
                Keep(current_);
                current_ = Onset{};
            }
            if(rise > current_.rise)
            {
                current_ = {
                    std::max(float(f * frame_) / float(length_) - pre_, 0.0f),
                    rise};
                frame_at_ = f;
            }
        }
        const size_t slot = f % kHistoryFrames;
        history_sum_ += power - history_[slot];
        history_[slot] = power;
    }
    next_ = last;
    if(next_ < frames_)
        return false;
    Keep(current_);
    current_ = Onset{};
    return true;
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
