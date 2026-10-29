#pragma once

#include <array>
#include "daisy_seed.h"

namespace clessidra {

// Three-position switches: S09 left (D9 up, D8 down), S07 right (D7 up, D6 down).
class Switches {
public:
    enum Switch : size_t { S09, S07, kNumSwitches };

    void init();
    void process();

    // daisy::Switch3::POS_UP, POS_CENTER or POS_DOWN
    int position(size_t s) const { return _positions[s]; }

private:
    std::array<daisy::Switch3, kNumSwitches> _switches;
    std::array<int, kNumSwitches> _positions{};
};

} // namespace clessidra
