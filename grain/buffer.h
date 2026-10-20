#pragma once

#include <cstddef>

namespace clessidra {

// Mono sample buffer in external memory. Recording replaces the content from the
// start, with short fades at both ends so the recorded material has no clicks at
// its edges.
class Buffer {
public:
    static constexpr float kFadeSeconds = 0.005f;

    // mem: capacity floats, zeroed here (SDRAM content is undefined at boot)
    void Init(float* mem, size_t capacity, float sample_rate);

    void StartRecording();
    void StopRecording(); // finishes after the fade-out

    // Every sample. Writes only while recording.
    void Write(float x);

    // Linear interpolation. pos wraps within Length().
    float Read(float pos) const;

    // Recorded samples. Updated when a recording finishes.
    size_t Length() const { return _length; }
    size_t Capacity() const { return _capacity; }
    bool IsRecording() const { return _state != State::Idle; }

private:
    enum class State { Idle, Recording, FadingOut };

    void _finish();

    float* _mem = nullptr;
    size_t _capacity = 0;
    size_t _length = 0;
    size_t _write_pos = 0;
    size_t _fade_samples = 1;
    size_t _fade_count = 0;
    State _state = State::Idle;
};

} // namespace clessidra
