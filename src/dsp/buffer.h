#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace clessidra {

// Mono recording in external memory, read by the grains. Filled by Ring::Copy(), with
// short fades at both ends so it has no clicks at its edges.
class Buffer {
public:
    static constexpr float kFadeSeconds = 0.005f;

    // mem: capacity floats, zeroed here (SDRAM content is undefined at boot)
    void Init(float* mem, size_t capacity, float sample_rate);

    // After writing length samples to Data(): sets the length and fades the ends
    void Finish(size_t length);
    float* Data() { return _mem; }
    const float* Data() const { return _mem; }
    size_t Capacity() const { return _capacity; }

    // Linear interpolation. pos wraps within Length().
    float Read(double pos) const;

    size_t Length() const { return _length; }

private:
    float* _mem = nullptr;
    size_t _capacity = 0;
    size_t _length = 0;
    size_t _fade_samples = 1;
};

// Mono ring in external memory, written every sample. A region of what was written
// is selected, trimmed of silence at both ends and copied to a Buffer, a part per call.
// The trim keeps a fade's length of silence, so the fades fall before the sound.
class Ring {
public:
    static constexpr float kSilence = 0.001f; // -60 dBFS

    enum class Step { Pending, Done, Silent };

    // mem: capacity floats
    void Init(float* mem, size_t capacity, float sample_rate);

    void Write(float x);

    // The region: length samples ending end_back samples before the next write, within
    // what was written, up to the Buffer's capacity. Writing on must not reach it
    // before the copy is done.
    void Select(size_t end_back, size_t length);
    // Copies to dst without the silence at both ends. At most max_samples per call.
    Step Copy(Buffer& dst, size_t max_samples);

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

// Inline: called per grain per sample
inline float Buffer::Read(double pos) const {
    if (_length == 0) return 0.0f;
    const double len = double(_length);
    if (pos < 0.0 || pos >= len) pos -= len * std::floor(pos / len);

    // pos can round up to len
    const size_t i0 = std::min(static_cast<size_t>(pos), _length - 1);
    const size_t i1 = (i0 + 1 < _length) ? i0 + 1 : 0;
    const float frac = float(pos - double(i0));
    return _mem[i0] + frac * (_mem[i1] - _mem[i0]);
}

} // namespace clessidra
