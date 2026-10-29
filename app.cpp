#include "app.h"

#include "daisy_seed.h"
#include "core/config.h"
#include "core/core.h"
#include "hw/touch.h"
#include "ui/core.ui.h"

namespace clessidra {

static constexpr auto     kSampleRate = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ; // kSampleRateHz
static constexpr size_t   kBlockSize  = 48;  // 1 ms at 48 kHz
static constexpr uint32_t kLoopMs     = 3;   // pad scan interval
static constexpr uint32_t kLogMs      = 250; // serial log interval (DEBUG=1)

static float DSY_SDRAM_BSS ring[kRingSamples];
static float DSY_SDRAM_BSS buffers[Core::kNumBuffers][kRecordSamples];

class AppImpl {
public:
    void Init();
    void Loop();
    void ProcessAudio(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size);

private:
#if DEBUG
    void _log();
#endif

    Touch _touch;
    Core _core;
    CoreUI _ui;
    daisy::CpuLoadMeter _cpu;
};

} // namespace clessidra

using namespace clessidra;

static AppImpl impl;

static void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
    impl.ProcessAudio(in, out, size);
}

void AppImpl::Init() {
    _touch.Init(kSampleRate, kBlockSize);
    daisy::DaisySeed& seed = _touch.seed();
    float* buffer_ptrs[Core::kNumBuffers];
    for (size_t b = 0; b < Core::kNumBuffers; b++) buffer_ptrs[b] = buffers[b];
    _core.Init(seed.AudioSampleRate(), ring, kRingSamples, buffer_ptrs, kRecordSamples);
    _cpu.Init(seed.AudioSampleRate(), seed.AudioBlockSize());
#if DEBUG
    seed.StartLog(false);
#endif
    seed.StartAudio(AudioCallback);
}

void AppImpl::Loop() {
#if DEBUG
    uint32_t last_log = daisy::System::GetNow();
#endif

    while (true) {
        _touch.ProcessPads();
        const Controls c = _touch.Read();
        _ui.Process(c, _core);
        _touch.SetLed(_ui.Led(c.now_ms, _core));

#if DEBUG
        if (daisy::System::GetNow() - last_log >= kLogMs) {
            last_log = daisy::System::GetNow();
            _log();
        }
#endif

        daisy::System::Delay(kLoopMs);
    }
}

void AppImpl::ProcessAudio(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
    _cpu.OnBlockStart();
    _touch.ProcessControls();
    _core.Process(in, out, size);
    _cpu.OnBlockEnd();
}

#if DEBUG
// Values as 0-100
static int Percent(float x) {
    return static_cast<int>(x * 100.0f + 0.5f);
}

void AppImpl::_log() {
    daisy::DaisySeed& seed = _touch.seed();
    seed.Print("knobs");
    for (size_t k = 0; k < Knobs::kNumKnobs; k++) {
        seed.Print(" %3d", Percent(_touch.knobs().Value(k)));
    }
    seed.Print(" | S09 %d S07 %d | pads",
               _touch.switches().Position(Switches::S09),
               _touch.switches().Position(Switches::S07));
    for (size_t p = 0; p < Pads::kNumPads; p++) {
        seed.Print(" %3d", Percent(_touch.pads().Pressure(p)));
    }
    seed.Print(" | rec %d len %d.%d s", _core.IsRecording(),
               static_cast<int>(_core.RecordedSeconds()),
               static_cast<int>(_core.RecordedSeconds() * 10.0f) % 10);
    seed.PrintLine(" | cpu avg %d max %d", Percent(_cpu.GetAvgCpuLoad()), Percent(_cpu.GetMaxCpuLoad()));
    // Readings per log line
    _cpu.Reset();
}
#endif

void Application::Init() {
    impl.Init();
}

void Application::Loop() {
    impl.Loop();
}
