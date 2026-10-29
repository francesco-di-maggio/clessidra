#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra {

// Mono ring in external memory, written every sample. A region of what was written
// is selected, trimmed of silence at both ends and copied to a Buffer, a part per call.
// The trim keeps a fade's length of silence, so the fades fall before the sound.
class Ring {
public:
    static constexpr float kSilence = 0.001f; // -60 dBFS

    enum class Step { Pending, Done, Silent };

    // mem: capacity floats
    void init(float* mem, size_t capacity, float sample_rate);

    void write(float x);

    // The region: length samples ending end_back samples before the next write, within
    // what was written, up to the Buffer's capacity. Writing on must not reach it
    // before the copy is done.
    void select(size_t end_back, size_t length);
    // Copies to dst without the silence at both ends. At most max_samples per call.
    Step copy(Buffer& dst, size_t max_samples);

private:
    size_t _index(size_t i) const { return (_start + i) % _capacity; }

    float* _mem = nullptr;
    size_t _capacity = 0;
    size_t _write_pos = 0;
    size_t _written = 0; // up to capacity
    size_t _start = 0;   // region
    size_t _length = 0;
    size_t _done = 0;    // region samples skipped or copied
    size_t _copied = 0;  // into dst; 0 while skipping silence
    size_t _last = 0;    // dst samples up to the last above kSilence
    size_t _margin = 1;  // samples kept around the sound
};

} // namespace clessidra
