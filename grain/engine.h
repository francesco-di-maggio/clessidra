#pragma once

#include <cstddef>

namespace clessidra {

// The instrument's signal path. No hardware code here.
class Engine {
public:
    void Init(float sample_rate);
    void Process(const float* const* in, float** out, size_t size);

private:
    float _sample_rate = 48000.0f;
};

} // namespace clessidra
