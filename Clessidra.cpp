#include "daisy_seed.h"
#include "config.h"
#include "hw/led.h"
#include "hw/touch.h"
#include "dsp/engine.h"
#include "ui/clessidra_ui.h"

// Read by the Synthux libDaisy fork's USB descriptors
const char* USBD_MANUFACTURER_STRING = CLESSIDRA_MANUFACTURER;
const char* USBD_PRODUCT_STRING_HS   = CLESSIDRA_PRODUCT;
const char* USBD_PRODUCT_STRING_FS   = CLESSIDRA_PRODUCT;

using namespace clessidra;

static daisy::DaisySeed    hw;
static Touch               touch;
static Engine              engine;
static ClessidraUI         ui;
static Led                 led;
static daisy::CpuLoadMeter cpu;

static float DSY_SDRAM_BSS ring[kRingSamples];
static float DSY_SDRAM_BSS buffers[Engine::kNumBuffers][kRecordSamples];

static void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
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
    led.Init();
    float* buffer_ptrs[Engine::kNumBuffers];
    for (size_t b = 0; b < Engine::kNumBuffers; b++) buffer_ptrs[b] = buffers[b];
    engine.Init(hw.AudioSampleRate(), ring, kRingSamples, buffer_ptrs, kRecordSamples);
    cpu.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

#ifdef CLESSIDRA_LOG
    hw.StartLog(false);
    uint32_t last_log = daisy::System::GetNow();
#endif

    hw.StartAudio(AudioCallback);

    while (1) {
        touch.ProcessPads();
        const Controls c = touch.Read();
        ui.Process(c, engine);
        led.Set(ui.Led(c.now_ms, engine));

#ifdef CLESSIDRA_LOG
        if (daisy::System::GetNow() - last_log >= kLogMs) {
            last_log = daisy::System::GetNow();
            LogState();
        }
#endif

        daisy::System::Delay(kLoopMs);
    }
}
