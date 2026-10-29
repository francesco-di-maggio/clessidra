#include "pads.h"

#include <algorithm>

using namespace daisy;

namespace clessidra {

void Pads::Init() {
    Mpr121I2C::Config cfg;
    cfg.touch_threshold = kTouchThreshold;
    cfg.release_threshold = kReleaseThreshold;
    _mpr.Init(cfg);
    _configure();
}

// Registers: NXP AN3889 (auto-configuration), AN3890 (analog front end), AN3891
// (baseline filters)
void Pads::_configure() {
    struct Setting {
        uint8_t reg;
        uint8_t value;
    };
    static constexpr Setting kSettings[] = {
        { Mpr121I2C::MPR121_ECR, 0x00 }, // stop mode
        // Rising data (finger lifting): baseline follows quickly
        { Mpr121I2C::MPR121_MHDR, 0x01 },
        { Mpr121I2C::MPR121_NHDR, 0x01 },
        { Mpr121I2C::MPR121_NCLR, 0x0E },
        { Mpr121I2C::MPR121_FDLR, 0x00 },
        // Falling data (finger pressing): baseline follows slowly, so a held press keeps its delta
        { Mpr121I2C::MPR121_MHDF, 0x01 },
        { Mpr121I2C::MPR121_NHDF, 0x01 },
        { Mpr121I2C::MPR121_NCLF, 0x10 },
        { Mpr121I2C::MPR121_FDLF, 0x04 },
        // Touched: baseline held
        { Mpr121I2C::MPR121_NHDT, 0x00 },
        { Mpr121I2C::MPR121_NCLT, 0x00 },
        { Mpr121I2C::MPR121_FDLT, 0x00 },
        // Touch and release each confirmed over 2 samples
        { Mpr121I2C::MPR121_DEBOUNCE, 0x11 },
        // 34 first-filter samples, 16 uA charge current
        { Mpr121I2C::MPR121_CONFIG1, 0xD0 },
        // 1 us charge time, 10 second-filter samples, 2 ms sample interval
        { Mpr121I2C::MPR121_CONFIG2, 0x51 },
        // Auto-configuration for Vdd = 3.3 V: upper (Vdd - 0.7) / Vdd * 256, target 0.9
        // of it, lower 0.65 of it. Sets each pad's charge current and time.
        { Mpr121I2C::MPR121_UPLIMIT, 200 },
        { Mpr121I2C::MPR121_LOWLIMIT, 130 },
        { Mpr121I2C::MPR121_TARGETLIMIT, 180 },
        { Mpr121I2C::MPR121_AUTOCONFIG0, 0xCB }, // 34 samples, baseline to target, enable, reconfigure
        { Mpr121I2C::MPR121_AUTOCONFIG1, 0x00 },
        // Run: baseline tracking from the first data, 12 pads
        { Mpr121I2C::MPR121_ECR, 0x8C },
    };
    for (const auto& s : kSettings) _mpr.WriteRegister(s.reg, s.value);
    System::Delay(80); // auto-configuration
}

void Pads::Process() {
    const uint16_t touched = _mpr.Touched();
    _touched = 0;
    for (size_t p = 0; p < kNumPads; p++) {
        const int32_t delta = int32_t(_mpr.BaselineData(p)) - int32_t(_mpr.FilteredData(p));
        // A delta back below the release threshold is a release, whatever the touch bit says
        const bool on = (touched & (1 << p)) != 0 && delta >= kReleaseThreshold;
        if (on) _touched |= uint16_t(1 << p);

        const float target = on ? std::clamp(float(delta - kReleaseThreshold) / float(kMaxDelta - kReleaseThreshold), 0.0f, 1.0f) : 0.0f;
        _pressure[p] += kSmoothing * (target - _pressure[p]);
        if (!on && _pressure[p] < kPressureFloor) _pressure[p] = 0.0f;
    }
}

} // namespace clessidra
