#include "daisy_seed.h"
#include "config.h"
#include "touch/touch.h"
#include "grain/engine.h"
#include "ui/clessidra_ui.h"

// Read by the Synthux libDaisy fork's USB descriptors
const char* USBD_MANUFACTURER_STRING = CLESSIDRA_MANUFACTURER;
const char* USBD_PRODUCT_STRING_HS   = CLESSIDRA_PRODUCT;
const char* USBD_PRODUCT_STRING_FS   = CLESSIDRA_PRODUCT;

using namespace daisy;
using namespace clessidra;

static DaisySeed    hw;
static Touch        touch;
static Engine       engine;
static ClessidraUI  ui;
static PWMHandle    led;
static CpuLoadMeter cpu;

static float DSY_SDRAM_BSS ring[kRingSamples];
static float DSY_SDRAM_BSS buffers[Engine::kNumBuffers][kRecordSamples];

// The panel as the UI reads it
static Controls ReadControls() {
    Controls c;
    for (size_t k = 0; k < Controls::kNumKnobs; k++) c.knobs[k] = touch.knobs().Value(k);
    c.switches[Controls::S09] = Controls::Position(touch.switches().Position(Switches::S09));
    c.switches[Controls::S07] = Controls::Position(touch.switches().Position(Switches::S07));
    for (size_t p = 0; p < Controls::kNumPads; p++) {
        c.touched[p] = touch.pads().IsTouched(p);
        c.pressure[p] = touch.pads().Pressure(p);
    }
    c.now_ms = System::GetNow();
    return c;
}

static void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    cpu.OnBlockStart();
    touch.ProcessControls();
    engine.Process(in, out, size);
    cpu.OnBlockEnd();
}

#ifdef CLESSIDRA_LOG
// Values as 0-100
static int Percent(float x) {
    return static_cast<int>(x * 100.0f + 0.5f);
}

static void LogState() {
    hw.Print("knobs");
    for (size_t k = 0; k < Knobs::kNumKnobs; k++) {
        hw.Print(" %3d", Percent(touch.knobs().Value(k)));
    }
    hw.Print(" | S09 %d S07 %d | pads",
             touch.switches().Position(Switches::S09),
             touch.switches().Position(Switches::S07));
    for (size_t p = 0; p < Pads::kNumPads; p++) {
        hw.Print(" %3d", Percent(touch.pads().Pressure(p)));
    }
    hw.Print(" | rec %d len %d.%d s", engine.IsRecording(),
             static_cast<int>(engine.RecordedSeconds()),
             static_cast<int>(engine.RecordedSeconds() * 10.0f) % 10);
    hw.PrintLine(" | cpu avg %d max %d", Percent(cpu.GetAvgCpuLoad()), Percent(cpu.GetMaxCpuLoad()));
    // Readings per log line
    cpu.Reset();
}
#endif

int main(void) {
    hw.Init(true);
    hw.SetAudioSampleRate(kSampleRate);
    hw.SetAudioBlockSize(kBlockSize);

    touch.Init(hw);
    // TIM3 channel 2 drives PC7, the Seed's LED. 16-bit period: about 3.7 kHz.
    led.Init(PWMHandle::Config(PWMHandle::Config::Peripheral::TIM_3));
    led.Channel2().Init();
    float* buffer_ptrs[Engine::kNumBuffers];
    for (size_t b = 0; b < Engine::kNumBuffers; b++) buffer_ptrs[b] = buffers[b];
    engine.Init(hw.AudioSampleRate(), ring, kRingSamples, buffer_ptrs, kRecordSamples);
    cpu.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

#ifdef CLESSIDRA_LOG
    hw.StartLog(false);
    uint32_t last_log = System::GetNow();
#endif

    hw.StartAudio(AudioCallback);

    while (1) {
        touch.ProcessPads();
        const Controls c = ReadControls();
        ui.Process(c, engine);
        led.Channel2().Set(ui.Led(c.now_ms, engine));

#ifdef CLESSIDRA_LOG
        if (System::GetNow() - last_log >= kLogMs) {
            last_log = System::GetNow();
            LogState();
        }
#endif

        System::Delay(kLoopMs);
    }
}
