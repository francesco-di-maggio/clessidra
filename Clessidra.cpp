#include <daisy_seed.h>
#include "core/config.h"
#include "core/core.h"
#include "hw/touch.h"
#include "ui/core.ui.h"

// Declared extern in lib/libDaisy/src/usbd/usbd_desc.c (Synthux-Academy fork).
// FS: the Seed's micro-USB. HS: pins 37, 38.
const char* USBD_MANUFACTURER_STRING = "Synthux";
const char* USBD_PRODUCT_STRING_HS   = "Clessidra";
const char* USBD_PRODUCT_STRING_FS   = "Clessidra";

namespace clessidra
{
namespace
{
    // kSampleRateHz
    constexpr auto kSampleRate
        = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
    constexpr size_t   kBlockSize = 48;  // 1 ms at 48 kHz
    constexpr uint32_t kLoopMs    = 3;   // pad scan interval
    constexpr uint32_t kLogMs     = 250; // serial log interval (DEBUG=1)

    float DSY_SDRAM_BSS ring[kRingSamples];
    float DSY_SDRAM_BSS buffers[Core::kNumBuffers][kRecordSamples];

    Touch               touch;
    Core                core;
    CoreUI              ui;
    daisy::CpuLoadMeter cpu;

    void AudioCallback(daisy::AudioHandle::InputBuffer  in,
                       daisy::AudioHandle::OutputBuffer out,
                       size_t                           size)
    {
        cpu.OnBlockStart();
        touch.ProcessControls();
        core.Process(in, out, size);
        cpu.OnBlockEnd();
    }

#if DEBUG
    // Values as 0-100
    int Percent(float x) { return static_cast<int>(x * 100.0f + 0.5f); }

    void Log(const Controls& c)
    {
        daisy::DaisySeed& seed = touch.seed;
        seed.Print("knobs");
        for(size_t k = 0; k < Controls::kNumKnobs; k++)
        {
            seed.Print(" %3d", Percent(c.knobs[k]));
        }
        seed.Print(" | S09 %d S07 %d | pads",
                   c.switches[Controls::S09],
                   c.switches[Controls::S07]);
        for(size_t p = 0; p < Controls::kNumPads; p++)
        {
            seed.Print(" %3d", Percent(c.pressure[p]));
        }
        seed.Print(" | rec %d len %d.%d s",
                   core.IsRecording(),
                   static_cast<int>(core.RecordedSeconds()),
                   static_cast<int>(core.RecordedSeconds() * 10.0f) % 10);
        seed.PrintLine(" | cpu avg %d max %d",
                       Percent(cpu.GetAvgCpuLoad()),
                       Percent(cpu.GetMaxCpuLoad()));
        // Readings per log line
        cpu.Reset();
    }
#endif

    void Init()
    {
        touch.Init(kSampleRate, kBlockSize);
        daisy::DaisySeed& seed = touch.seed;
        float*            buffer_ptrs[Core::kNumBuffers];
        for(size_t b = 0; b < Core::kNumBuffers; b++)
            buffer_ptrs[b] = buffers[b];
        core.Init(seed.AudioSampleRate(),
                  ring,
                  kRingSamples,
                  buffer_ptrs,
                  kRecordSamples);
        cpu.Init(seed.AudioSampleRate(), seed.AudioBlockSize());
#if DEBUG
        seed.StartLog(false);
#endif
        seed.StartAudio(AudioCallback);
    }

    void Loop()
    {
#if DEBUG
        uint32_t last_log = daisy::System::GetNow();
#endif

        while(true)
        {
            touch.ProcessPads();
            const Controls c = touch.Read();
            ui.Process(c, core);
            touch.SetLed(ui.Led(c.now_ms, core));

#if DEBUG
            if(c.now_ms - last_log >= kLogMs)
            {
                last_log = c.now_ms;
                Log(c);
            }
#endif

            daisy::System::Delay(kLoopMs);
        }
    }

} // namespace
} // namespace clessidra

int main(void)
{
    clessidra::Init();
    clessidra::Loop();
}
