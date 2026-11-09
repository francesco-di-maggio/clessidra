#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra
{
// Mono ring in external memory, written every sample: the input, always
// listening. A region of what was written is selected, trimmed of silence at
// both ends and copied to a Buffer, a part per call. The trim keeps a fade's
// length of silence, so the fades fall before the sound.
class Ring
{
  public:
    static constexpr float kSilence = 0.001f; // -60 dBFS

    enum class Step
    {
        Pending,
        Done,
        Silent
    };

    // mem: capacity floats, zeroed here
    void Init(float* mem, size_t capacity, float sample_rate);

    void Write(float x)
    {
        mem_[write_] = x;
        if(++write_ == capacity_)
            write_ = 0;
        if(written_ < capacity_)
            written_++;
    }

    size_t Capacity() const { return capacity_; }
    // For grains reading it live: its memory, and where the next sample goes
    float* Data() { return mem_; }
    size_t WriteIndex() const { return write_; }

    // The region: length samples ending end_back samples before the next
    // write, within what was written. Writing on must not reach it before the
    // copy is done.
    void Select(size_t end_back, size_t length);
    // Copies to dst without the silence at both ends, at most max_samples per
    // call, up to dst's capacity
    Step Copy(Buffer& dst, size_t max_samples);

  private:
    size_t Index(size_t i) const { return (start_ + i) % capacity_; }

    float* mem_      = nullptr;
    size_t capacity_ = 0;
    size_t write_    = 0;
    size_t written_  = 0; // up to capacity
    size_t start_    = 0; // region
    size_t length_   = 0;
    size_t done_     = 0; // region samples skipped or copied
    size_t copied_   = 0; // into dst, 0 while skipping silence
    size_t last_     = 0; // dst samples up to the last above kSilence
    size_t margin_   = 1; // samples kept around the sound
};

} // namespace clessidra
