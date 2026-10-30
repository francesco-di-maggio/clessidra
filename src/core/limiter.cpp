#include "limiter.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
void Limiter::Init(float sample_rate)
{
    sample_rate = std::min(sample_rate, kMaxSampleRate);
    const size_t box
        = std::max<size_t>(1, size_t(sample_rate * kAttackSeconds / 2.0f));
    box1_.Init(box);
    box2_.Init(box);
    // The two boxes average over 2 * box - 1 samples: the audio is delayed to
    // the newest of them, the minimum window covers them all plus the hold.
    const size_t delay = 2 * box - 2;
    min_.Init(2 * box - 1 + size_t(sample_rate * kHoldSeconds));
    release_       = 1.0f;
    release_coeff_ = 1.0f - std::exp(-1.0f / (kReleaseSeconds * sample_rate));
    delay_l_.Init();
    delay_r_.Init();
    delay_l_.SetDelay(delay + 1);
    delay_r_.SetDelay(delay + 1);
}

void Limiter::Process(float& left, float& right)
{
    const float peak   = std::max(std::fabs(left), std::fabs(right));
    const float target = peak > kThreshold ? kThreshold / peak : 1.0f;

    const float hold = min_.Process(target);
    // Falls at once, rises exponentially
    release_ = std::min(hold, release_ + release_coeff_ * (hold - release_));
    const float gain = box2_.Process(box1_.Process(release_));

    delay_l_.Write(left);
    delay_r_.Write(right);
    left  = delay_l_.Read() * gain;
    right = delay_r_.Read() * gain;
}

void Limiter::MovingMin::Init(size_t length)
{
    length_ = std::clamp<size_t>(length, 1, kMaxWindow);
    front_  = 0;
    count_  = 0;
    now_    = 0;
}

float Limiter::MovingMin::Process(float x)
{
    // Expire before pushing: at most length_ values in the window
    if(count_ > 0 && now_ - time_[front_] >= length_)
    {
        front_ = (front_ + 1) % length_;
        count_--;
    }
    // Values not below x can never be the minimum again
    while(count_ > 0 && value_[(front_ + count_ - 1) % length_] >= x)
        count_--;
    const size_t back = (front_ + count_) % length_;
    value_[back]      = x;
    time_[back]       = now_;
    count_++;
    now_++;
    return value_[front_];
}

void Limiter::Box::Init(size_t length)
{
    length_ = std::clamp<size_t>(length, 1, kMaxBox);
    std::fill(ring_.begin(), ring_.begin() + length_, 1.0f);
    inv_length_ = 1.0 / double(length_);
    pos_        = 0;
    sum_        = double(length_);
}

float Limiter::Box::Process(float x)
{
    sum_ += double(x) - double(ring_[pos_]);
    ring_[pos_] = x;
    pos_        = (pos_ + 1) % length_;
    return float(sum_ * inv_length_);
}

} // namespace clessidra
