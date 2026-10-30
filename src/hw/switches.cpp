#include "switches.h"

namespace clessidra
{
void Switches::Init()
{
    switches_[S09].Init(daisy::seed::D9, daisy::seed::D8);
    switches_[S07].Init(daisy::seed::D7, daisy::seed::D6);
}

void Switches::Process()
{
    for(size_t s = 0; s < kNumSwitches; s++)
    {
        positions_[s] = switches_[s].Read();
    }
}

} // namespace clessidra
