#pragma once

#include <daisy_seed.h>
#include "controls.h"
#include "knobs.h"
#include "pads.h"
#include "switches.h"

namespace clessidra
{
// The Simple Touch: the Seed, its LED and the panel, read with libDaisy's own
// drivers.
class Touch
{
  public:
    Touch()             = default;
    Touch(const Touch&) = delete;
    Touch& operator=(const Touch&) = delete;

    void Init(daisy::SaiHandle::Config::SampleRate sample_rate,
              size_t                               block_size)
    {
        seed.Init(true);
        seed.SetAudioSampleRate(sample_rate);
        seed.SetAudioBlockSize(block_size);
        // Knobs are smoothed at the callback rate, set above
        knobs_.Init(seed, seed.AudioCallbackRate());
        seed.adc.Start();
        switches_.Init();
        pads_.Init();
        // TIM3 channel 2 drives PC7, the Seed's LED. 16-bit period: about 3.7
        // kHz.
        led_.Init(daisy::PWMHandle::Config(
            daisy::PWMHandle::Config::Peripheral::TIM_3));
        led_.Channel2().Init();
    }

    // Audio callback, once per block.
    void ProcessControls()
    {
        knobs_.Process();
        switches_.Process();
    }

    // Main loop.
    void ProcessPads() { pads_.Process(); }

    // The panel as the UI reads it
    Controls Read() const
    {
        Controls c;
        for(size_t k = 0; k < Controls::kNumKnobs; k++)
            c.knobs[k] = knobs_.Value(k);
        c.switches[Controls::S09]
            = Controls::Position(switches_.Position(Switches::S09));
        c.switches[Controls::S07]
            = Controls::Position(switches_.Position(Switches::S07));
        for(size_t p = 0; p < Controls::kNumPads; p++)
        {
            c.touched[p]  = pads_.IsTouched(p);
            c.pressure[p] = pads_.Pressure(p);
        }
        c.now_ms = daisy::System::GetNow();
        return c;
    }

    // 0..1, duty cycle
    void SetLed(float value) { led_.Channel2().Set(value); }

    daisy::DaisySeed seed;

  private:
    daisy::PWMHandle led_;
    Knobs            knobs_;
    Pads             pads_;
    Switches         switches_;
};

} // namespace clessidra
