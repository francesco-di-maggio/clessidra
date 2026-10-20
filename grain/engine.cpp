#include "engine.h"

namespace clessidra {

void Engine::Init(float sample_rate) {
    _sample_rate = sample_rate;
}

void Engine::Process(const float* const* in, float** out, size_t size) {
    for (size_t i = 0; i < size; i++) {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}

} // namespace clessidra
