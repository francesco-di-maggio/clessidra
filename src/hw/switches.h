#pragma once

#include <array>
#include <daisy_seed.h>

namespace clessidra
{
// Three-position switches: S09 left (D9 up, D8 down), S07 right (D7 up, D6
// down).
class Switches
{
  public:
    enum Switch : size_t
    {
        S09,
        S07,
        kNumSwitches
    };

    void Init();
    void Process();

    // daisy::Switch3::POS_UP, POS_CENTER or POS_DOWN
    int Position(size_t s) const { return positions_[s]; }

  private:
    std::array<daisy::Switch3, kNumSwitches> switches_;
    std::array<int, kNumSwitches>            positions_{};
};

} // namespace clessidra
