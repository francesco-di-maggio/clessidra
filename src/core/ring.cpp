#include "ring.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
void Ring::Init(float* mem, size_t capacity, float sample_rate)
{
    mem_      = mem;
    capacity_ = capacity;
    margin_   = std::max<size_t>(
        1, static_cast<size_t>(Buffer::kFadeSeconds * sample_rate));
    std::fill(mem_, mem_ + capacity_, 0.0f);
}

void Ring::Write(float x)
{
    mem_[write_pos_] = x;
    if(++write_pos_ == capacity_)
        write_pos_ = 0;
    written_ = std::min(written_ + 1, capacity_);
}

void Ring::Select(size_t end_back, size_t length)
{
    end_back = std::min(end_back, written_);
    length_  = std::min(length, written_ - end_back);
    start_   = (write_pos_ + 2 * capacity_ - end_back - length_) % capacity_;
    done_    = 0;
    copied_  = 0;
    last_    = 0;
}

Ring::Step Ring::Copy(Buffer& dst, size_t max_samples)
{
    // At most the region: copied_ <= done_ <= length_ <= dst capacity
    const size_t end  = std::min(done_ + max_samples, length_);
    float*       data = dst.Data();
    size_t       i    = Index(done_);
    // Leading silence, then from the margin before the first sound
    for(; copied_ == 0 && done_ < end; done_++)
    {
        if(std::fabs(mem_[i]) > kSilence)
        {
            for(size_t k = done_ > margin_ ? done_ - margin_ : 0; k < done_;
                k++)
                data[copied_++] = mem_[Index(k)];
            break;
        }
        if(++i == capacity_)
            i = 0;
    }
    for(; done_ < end; done_++)
    {
        const float x = mem_[i];
        if(++i == capacity_)
            i = 0;
        data[copied_++] = x;
        if(std::fabs(x) > kSilence)
            last_ = copied_;
    }
    if(done_ < length_)
        return Step::Pending;
    if(copied_ == 0)
        return Step::Silent;
    dst.Finish(std::min(copied_, last_ + margin_));
    return Step::Done;
}

} // namespace clessidra
