#include "switches.h"

using namespace daisy;
using namespace daisy::seed;

namespace clessidra {

void Switches::Init() {
    _switches[S09].Init(D9, D8);
    _switches[S07].Init(D7, D6);
}

void Switches::Process() {
    for (size_t s = 0; s < kNumSwitches; s++) {
        _positions[s] = _switches[s].Read();
    }
}

} // namespace clessidra
