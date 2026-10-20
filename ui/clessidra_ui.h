#pragma once

#include "daisy_seed.h"
#include "../touch/touch.h"
#include "../grain/engine.h"

namespace clessidra {

// Maps the Touch controls to the engine and drives the LED.
class ClessidraUI {
public:
    ClessidraUI(Touch& touch, Engine& engine) : _touch{ touch }, _engine{ engine } {}

    // Main loop, after the pads are read.
    void Process(daisy::DaisySeed& hw);

private:
    Touch& _touch;
    Engine& _engine;
};

} // namespace clessidra
