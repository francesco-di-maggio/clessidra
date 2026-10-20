#pragma once

#include <cstddef>
#include "buffer.h"

namespace clessidra {

// The instrument's signal path. No hardware code here.
class Engine {
public:
    // buffer: buffer_size floats for the recording, allocated by the caller
    void Init(float sample_rate, float* buffer, size_t buffer_size);
    void Process(const float* const* in, float** out, size_t size);

    // Main loop. Applied at the next audio block.
    void SetRecording(bool on) { _record = on; }

    bool IsRecording() const { return _buffer.IsRecording(); }
    float RecordedSeconds() const { return float(_buffer.Length()) / _sample_rate; }

private:
    Buffer _buffer;
    float _sample_rate = 48000.0f;
    float _play_pos = 0.0f; // test loop until the grain engine
    volatile bool _record = false;
    bool _was_record = false;
};

} // namespace clessidra
