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
    void init();
    void loop();
    void process_audio(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size);

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
    impl.process_audio(in, out, size);
}

void AppImpl::init() {
    _touch.init(kSampleRate, kBlockSize);
    daisy::DaisySeed& seed = _touch.seed();
    float* buffer_ptrs[Core::kNumBuffers];
    for (size_t b = 0; b < Core::kNumBuffers; b++) buffer_ptrs[b] = buffers[b];
    _core.init(seed.AudioSampleRate(), ring, kRingSamples, buffer_ptrs, kRecordSamples);
    _cpu.Init(seed.AudioSampleRate(), seed.AudioBlockSize());
#if DEBUG
    seed.StartLog(false);
#endif
    seed.StartAudio(AudioCallback);
}

void AppImpl::loop() {
#if DEBUG
    uint32_t last_log = daisy::System::GetNow();
#endif

    while (true) {
        _touch.process_pads();
        const Controls c = _touch.read();
        _ui.process(c, _core);
        _touch.set_led(_ui.led(c.now_ms, _core));

#if DEBUG
        if (daisy::System::GetNow() - last_log >= kLogMs) {
            last_log = daisy::System::GetNow();
            _log();
        }
#endif

        daisy::System::Delay(kLoopMs);
    }
}

void AppImpl::process_audio(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
    _cpu.OnBlockStart();
    _touch.process_controls();
    _core.process(in, out, size);
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
        seed.Print(" %3d", Percent(_touch.knobs().value(k)));
    }
    seed.Print(" | S09 %d S07 %d | pads",
               _touch.switches().position(Switches::S09),
               _touch.switches().position(Switches::S07));
    for (size_t p = 0; p < Pads::kNumPads; p++) {
        seed.Print(" %3d", Percent(_touch.pads().pressure(p)));
    }
    seed.Print(" | rec %d len %d.%d s", _core.is_recording(),
               static_cast<int>(_core.recorded_seconds()),
               static_cast<int>(_core.recorded_seconds() * 10.0f) % 10);
    seed.PrintLine(" | cpu avg %d max %d", Percent(_cpu.GetAvgCpuLoad()), Percent(_cpu.GetMaxCpuLoad()));
    // Readings per log line
    _cpu.Reset();
}
#endif

void Application::init() {
    impl.init();
}

void Application::loop() {
    impl.loop();
}
