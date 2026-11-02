// Clessidra in the browser: the board's controls, mapping and core behind a
// plain C interface, compiled to WebAssembly by build.sh. The page's
// AudioWorklet calls it once per render quantum.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include "core/config.h"
#include "core/core.h"
#include "hw/controls.h"
#include "ui/core.ui.h"

using namespace clessidra;

#define EXPORT extern "C" __attribute__((used, visibility("default")))

namespace
{
constexpr size_t kMaxBlock = 128; // Web Audio's render quantum
// Waveform columns over the buffer, each its peak level
constexpr size_t kColumns          = 1024;
constexpr size_t kSamplesPerColumn = kBufferSamples / kColumns;
static_assert(kBufferSamples % kColumns == 0);

float    sound[kBufferSamples];
Core     core;
CoreUI   ui;
Controls controls;
float    in_l[kMaxBlock], in_r[kMaxBlock], out_l[kMaxBlock], out_r[kMaxBlock];
float    peaks[kColumns];
size_t   last_write = 0;

// What the page draws, in samples of the buffer unless noted
enum View : size_t
{
    kWrite,
    kHeldEnd,
    kSound,
    kSelectionStart,
    kSelectionLength,
    kLiveStart,
    kLiveLength,
    kPlayhead,
    kLivePlayhead,
    kSlice, // kNumSlices: the whole selection
    kSliceEdges,
    kNumView = kSliceEdges + kNumSlices + 1
};
float view[kNumView];

// Per grain: position 0..1 of the buffer, progress 0..1, window shape, pan
constexpr size_t kGrainFields = 4;
float            grains[Core::kNumVoices * kGrainFields];

// The parameters in use, in the core's units, for the readout
enum Param : size_t
{
    kSpeed,
    kFreeze,
    kEnd,
    kDensity,
    kGrainMs,
    kPitch,
    kFill,
    kReverb,
    kJitter,
    kRandPitch,
    kWindowShape,
    kStart,
    kLength,
    kNumParams
};
float params[kNumParams];

void UpdatePeaks(size_t from, size_t to)
{
    for(size_t column = from / kSamplesPerColumn;
        column <= std::min(to / kSamplesPerColumn, kColumns - 1);
        column++)
    {
        float        peak  = 0.0f;
        const size_t first = column * kSamplesPerColumn;
        for(size_t i = first; i < first + kSamplesPerColumn; i++)
            peak = std::max(peak, std::fabs(sound[i]));
        peaks[column] = peak;
    }
}

} // namespace

EXPORT int cl_init(float sample_rate)
{
    // The mapping counts in kSampleRateHz, as on the board
    if(sample_rate != float(kSampleRateHz))
        return 0;
    core.Init(sample_rate, sound, kBufferSamples);
    last_write = core.WritePoint();
    return 1;
}

EXPORT float* cl_input(int channel)
{
    return channel == 0 ? in_l : in_r;
}
EXPORT float* cl_output(int channel)
{
    return channel == 0 ? out_l : out_r;
}

// One block: the mapping reads the controls, then the core runs, as the
// board's loop and audio callback do
EXPORT void cl_process(int size, uint32_t now_ms)
{
    controls.now_ms = now_ms;
    ui.Process(controls, core);
    const float* in[2]  = {in_l, in_r};
    float*       out[2] = {out_l, out_r};
    core.Process(in, out, std::min(size_t(size), kMaxBlock));

    const size_t write = core.WritePoint();
    if(write == last_write)
        return;
    if(write > last_write)
        UpdatePeaks(last_write, write);
    else
    {
        UpdatePeaks(last_write, kBufferSamples - 1);
        UpdatePeaks(0, write);
    }
    last_write = write;
}

// Controls, as Controls holds them: knobs 0..1, switches as libDaisy's
// Switch3, pads touched with a pressure 0..1
EXPORT void cl_knob(int knob, float value)
{
    controls.knobs[size_t(knob) % Controls::kNumKnobs] = value;
}
EXPORT void cl_switch(int which, int position)
{
    controls.switches[size_t(which) % Controls::kNumSwitches]
        = Controls::Position(position);
}
EXPORT void cl_pad(int pad, int touched, float pressure)
{
    const size_t p       = size_t(pad) % Controls::kNumPads;
    controls.touched[p]  = touched != 0;
    controls.pressure[p] = pressure;
}

EXPORT float cl_led(uint32_t now_ms)
{
    return ui.Led(now_ms, core);
}

EXPORT const float* cl_peaks()
{
    return peaks;
}
EXPORT int cl_columns()
{
    return int(kColumns);
}
EXPORT int cl_buffer_samples()
{
    return int(kBufferSamples);
}

// The selections as Core::Process places them
EXPORT const float* cl_view()
{
    const Core::Params& p      = core.CurrentParams();
    const double        sound  = double(core.SoundSamples());
    const double        buffer = double(kBufferSamples);
    const double        write  = double(core.WritePoint());
    const double held_end = p.fill >= 1.0f ? write : double(core.HeldEnd());
    const double lag = double(size_t(Core::kStartLagSeconds * kSampleRateHz));
    const double sel_length  = std::max(double(p.length) * sound, 1.0);
    const double sel_start   = held_end - double(p.start) * sound - sel_length;
    const double live_length = std::clamp(
        double(p.length) * buffer, 1.0, std::max(sound - lag, 1.0));
    const double live_start = write - lag - live_length;

    view[kWrite]           = float(write);
    view[kHeldEnd]         = float(held_end);
    view[kSound]           = float(sound);
    view[kSelectionStart]  = float(sel_start);
    view[kSelectionLength] = float(sel_length);
    view[kLiveStart]       = float(live_start);
    view[kLiveLength]      = float(live_length);
    view[kPlayhead]     = float(sel_start + core.PlayheadPhase() * sel_length);
    view[kLivePlayhead] = float(live_start + core.LivePhase() * live_length);
    view[kSlice]        = float(ui.Slice());
    for(size_t i = 0; i <= kNumSlices; i++)
        view[kSliceEdges + i] = float(ui.Slices()[i] * sound);
    return view;
}

// Fills the grain fields, returns how many grains are playing
EXPORT int cl_grains()
{
    int count = 0;
    for(const Grain& g : core.Grains())
    {
        if(!g.IsActive())
            continue;
        double position = g.Position() / double(kBufferSamples);
        position -= std::floor(position);
        float* fields = grains + size_t(count) * kGrainFields;
        fields[0]     = float(position);
        fields[1]     = float(g.Progress());
        fields[2]     = g.Settings().shape;
        fields[3]     = g.Settings().pan;
        count++;
    }
    return count;
}
EXPORT const float* cl_grain_fields()
{
    return grains;
}

EXPORT const float* cl_params()
{
    const Core::Params& p = core.CurrentParams();
    params[kSpeed]        = p.speed;
    params[kFreeze]       = p.freeze ? 1.0f : 0.0f;
    params[kEnd]          = float(p.end);
    params[kDensity]      = p.density;
    params[kGrainMs]      = p.grain_ms;
    params[kPitch]        = 12.0f * std::log2(p.pitch);
    params[kFill]         = p.fill;
    params[kReverb]       = p.reverb;
    params[kJitter]       = p.jitter;
    params[kRandPitch]    = p.rand_pitch;
    params[kWindowShape]  = p.window_shape;
    params[kStart]        = p.start;
    params[kLength]       = p.length;
    return params;
}

EXPORT float cl_grain_level()
{
    return core.GrainLevel();
}
