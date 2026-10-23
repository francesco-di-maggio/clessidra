// Clessidra in the browser: the engine and the Simple Touch mapping behind a plain C
// interface, compiled to WebAssembly by build.sh. The page's AudioWorklet calls it.

#include <cmath>
#include <cstdint>
#include <vector>
#include "../ui/clessidra_ui.h"

using namespace clessidra;

#define EXPORT extern "C" __attribute__((used, visibility("default")))

namespace {

constexpr size_t kRecordSeconds = 8;  // as on the board (config.h)
constexpr size_t kMaxBlock = 128;     // Web Audio's render quantum

Engine engine;
ClessidraUI ui;
Controls controls;
std::vector<float> ring;
std::vector<float> buffers[Engine::kNumBuffers];
float in_l[kMaxBlock], in_r[kMaxBlock], out_l[kMaxBlock], out_r[kMaxBlock];
// What the display reads: per grain, position 0..1 of the recording, progress, pan
float grain_view[Engine::kNumVoices * 3];

} // namespace

EXPORT void cl_init(float sample_rate) {
    const size_t record = kRecordSeconds * size_t(sample_rate);
    ring.assign(record + size_t(sample_rate), 0.0f); // 1 s more, as on the board
    float* ptrs[Engine::kNumBuffers];
    for (size_t b = 0; b < Engine::kNumBuffers; b++) {
        buffers[b].assign(record, 0.0f);
        ptrs[b] = buffers[b].data();
    }
    engine.Init(sample_rate, ring.data(), ring.size(), ptrs, record);
}

// Audio: 128 samples in, 128 out, through these buffers
EXPORT float* cl_input(int channel) { return channel == 0 ? in_l : in_r; }
EXPORT float* cl_output(int channel) { return channel == 0 ? out_l : out_r; }

// One block: the mapping reads the controls, then the engine runs
EXPORT void cl_process(int size, uint32_t now_ms) {
    controls.now_ms = now_ms;
    ui.Process(controls, engine);
    const float* in[2] = { in_l, in_r };
    float* out[2] = { out_l, out_r };
    engine.Process(in, out, size_t(size) <= kMaxBlock ? size_t(size) : kMaxBlock);
}

// Controls, as the panel's
EXPORT void cl_knob(int knob, float value) { controls.knobs[size_t(knob) % Controls::kNumKnobs] = value; }
EXPORT void cl_second_layer(int knob, float value) { ui.SetSecondLayer(size_t(knob), value); }
EXPORT float cl_second_layer_value(int knob) { return ui.SecondLayer(size_t(knob)); }
EXPORT void cl_switch(int sw, int position) {
    controls.switches[size_t(sw) % Controls::kNumSwitches] = Controls::Position(position);
}
EXPORT void cl_pad(int pad, int touched, float pressure) {
    const size_t p = size_t(pad) % Controls::kNumPads;
    controls.touched[p] = touched != 0;
    controls.pressure[p] = pressure;
}

EXPORT float cl_led(uint32_t now_ms) { return ui.Led(now_ms, engine); }

// Display
EXPORT const float* cl_recording() { return engine.Recording().Data(); }
EXPORT int cl_recording_length() { return int(engine.Recording().Length()); }
EXPORT float cl_selection_start() { return engine.CurrentParams().start; }
EXPORT float cl_selection_length() { return engine.CurrentParams().length; }
EXPORT double cl_source_phase(int source) { return engine.SourcePhase(size_t(source) % Engine::kNumSources); }
EXPORT int cl_source_on(int source) { return engine.SourceOn(size_t(source) % Engine::kNumSources) ? 1 : 0; }

// Fills the grain view; returns how many grains are playing
EXPORT int cl_grains() {
    const double length = double(engine.Recording().Length());
    int count = 0;
    for (const Grain& g : engine.Grains()) {
        if (!g.IsActive() || length <= 0.0) continue;
        double pos = g.Position() / length;
        pos -= std::floor(pos); // wrap to 0..1
        grain_view[count * 3 + 0] = float(pos);
        grain_view[count * 3 + 1] = g.Progress();
        grain_view[count * 3 + 2] = g.Pan();
        count++;
    }
    return count;
}
EXPORT const float* cl_grain_view() { return grain_view; }
