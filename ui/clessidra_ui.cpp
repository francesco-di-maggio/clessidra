#include "clessidra_ui.h"
#include "../config.h"

namespace clessidra {

void ClessidraUI::Process(daisy::DaisySeed& hw) {
    _engine.SetRecording(_touch.pads().IsTouched(kRecordPad));
    hw.SetLed(_engine.IsRecording());
}

} // namespace clessidra
