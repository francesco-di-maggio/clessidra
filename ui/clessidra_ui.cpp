#include "clessidra_ui.h"

namespace clessidra {

void ClessidraUI::Process(daisy::DaisySeed& hw) {
    hw.SetLed(_touch.pads().AnyTouched());
}

} // namespace clessidra
