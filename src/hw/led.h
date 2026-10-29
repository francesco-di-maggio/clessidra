#pragma once

#include "daisy_seed.h"

namespace clessidra {

// The Seed's LED (PC7) on TIM3 channel 2 PWM. 16-bit period: about 3.7 kHz.
class Led {
public:
    void Init() {
        _pwm.Init(daisy::PWMHandle::Config(daisy::PWMHandle::Config::Peripheral::TIM_3));
        _pwm.Channel2().Init();
    }

    // 0..1, duty cycle
    void Set(float value) { _pwm.Channel2().Set(value); }

private:
    daisy::PWMHandle _pwm;
};

} // namespace clessidra
