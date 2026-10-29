#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace clessidra {

// The Simple Touch panel as plain values, as the board reads them. No hardware code:
// the host tests fill it too.
struct Controls {
    enum Knob : size_t { S30, S31, S32, S33, S34, S35, S36, S37, kNumKnobs }; // S36, S37: faders
    enum Switch : size_t { S09, S07, kNumSwitches };
    enum Position : int { Centre, Up, Down }; // as libDaisy's Switch3
    static constexpr size_t kNumPads = 12; // P00-P11

    std::array<float, kNumKnobs> knobs{};          // 0..1
    std::array<Position, kNumSwitches> switches{}; // centre
    std::array<bool, kNumPads> touched{};
    std::array<float, kNumPads> pressure{};        // 0..1
    uint32_t now_ms = 0;
};

} // namespace clessidra
