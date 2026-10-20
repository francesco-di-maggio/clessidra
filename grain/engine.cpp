#include "engine.h"

namespace clessidra {

void Engine::Init(float sample_rate, float* buffer, size_t buffer_size) {
    _sample_rate = sample_rate;
    _buffer.Init(buffer, buffer_size, sample_rate);
}

void Engine::Process(const float* const* in, float** out, size_t size) {
    // Edges only: a recording that fills the buffer stays stopped while P10 is held.
    const bool record = _record;
    if (record && !_was_record) _buffer.StartRecording();
    if (!record && _was_record) _buffer.StopRecording();
    _was_record = record;

    for (size_t i = 0; i < size; i++) {
        const float mono = 0.5f * (in[0][i] + in[1][i]);
        _buffer.Write(mono);

        // Test loop: plays the whole recording, silent while recording.
        float play = 0.0f;
        if (!_buffer.IsRecording() && _buffer.Length() > 0) {
            play = _buffer.Read(_play_pos);
            _play_pos += 1.0f;
            if (_play_pos >= float(_buffer.Length())) _play_pos = 0.0f;
        } else {
            _play_pos = 0.0f;
        }

        out[0][i] = in[0][i] + play;
        out[1][i] = in[1][i] + play;
    }
}

} // namespace clessidra
