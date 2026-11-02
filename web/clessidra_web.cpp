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
// Waveform columns over the recording, each its peak level, a few per block
constexpr size_t kColumns         = 1200;
constexpr size_t kColumnsPerBlock = 40;

float    ring[kRingSamples];
float    sound[Core::kNumBuffers][kBufferSamples];
Core     core;
CoreUI   ui;
Controls controls;
float    in_l[kMaxBlock], in_r[kMaxBlock], out_l[kMaxBlock], out_r[kMaxBlock];
float    peaks[kColumns];
size_t   peaks_done = kColumns; // columns of the recording playing
uint32_t recordings = 0;

// What the page draws, in samples of the recording unless noted
enum View : size_t
{
    kSound,
    kSelectionStart,
    kSelectionLength,
    kPlayhead,
    kSliceEdges,
    kTriggers = kSliceEdges + kNumSlices + 1, // where each is, -1 when silent
    kNumView  = kTriggers + Core::kNumTriggers
};
float view[kNumView];

// Per grain: position 0..1 of the recording, progress 0..1, window shape, pan
constexpr size_t kGrainFields = 4;
float            grains[Core::kNumVoices * kGrainFields];

// The parameters in use, in the core's units, for the readout
enum Param : size_t
{
    kSpeed,
    kFreeze,
    kDensity,
    kGrainMs,
    kPitch,
    kReverb,
    kJitter,
    kRandPitch,
    kWindowShape,
    kStart,
    kLength,
    kMix,
    kFilter,
    kRandPos,
    kRandSize,
    kRandShape,
    kNumParams
};
float params[kNumParams];

void UpdatePeaks()
{
    const Buffer& b    = core.Sound();
    const size_t  last = std::min(peaks_done + kColumnsPerBlock, kColumns);
    for(; peaks_done < last; peaks_done++)
    {
        const size_t from = peaks_done * b.Length() / kColumns;
        const size_t to   = (peaks_done + 1) * b.Length() / kColumns;
        float        peak = 0.0f;
        for(size_t i = from; i < to; i++)
            peak = std::max(peak, std::fabs(b.Data()[i]));
        peaks[peaks_done] = peak;
    }
}

} // namespace

EXPORT int cl_init(float sample_rate)
{
    // The mapping counts in kSampleRateHz, as on the board
    if(sample_rate != float(kSampleRateHz))
        return 0;
    float* const buffers[] = {sound[0], sound[1]};
    core.Init(sample_rate, ring, kRingSamples, buffers, kBufferSamples);
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

    if(core.Recordings() != recordings)
    {
        recordings = core.Recordings();
        peaks_done = 0;
    }
    UpdatePeaks();
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

// A knob's second function, set directly: the page has a control for each
EXPORT void cl_second(int knob, float value)
{
    ui.SetSecond(Controls::Knob(size_t(knob) % Controls::kNumKnobs), value);
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
EXPORT int cl_recordings()
{
    return int(recordings);
}

// The selection as Core::Process places it
EXPORT const float* cl_view()
{
    const Core::Params& p          = core.CurrentParams();
    const double        sound      = double(core.SoundSamples());
    const double        sel_length = std::max(double(p.length) * sound, 1.0);
    const double        sel_start  = double(p.start) * sound;

    view[kSound]           = float(sound);
    view[kSelectionStart]  = float(sel_start);
    view[kSelectionLength] = float(sel_length);
    view[kPlayhead] = float(sel_start + core.PlayheadPhase() * sel_length);
    for(size_t i = 0; i <= kNumSlices; i++)
        view[kSliceEdges + i] = float(ui.Slices()[i] * sound);
    for(size_t i = 0; i < Core::kNumTriggers; i++)
    {
        const Core::Trigger& s = p.triggers[i];
        view[kTriggers + i]
            = core.TriggerOn(i) ? float(
                  (s.start + core.TriggerPhase(i) * (s.end - s.start)) * sound)
                                : -1.0f;
    }
    return view;
}

// Fills the grain fields with the grains reading the recording playing,
// returns how many
EXPORT int cl_grains()
{
    const Buffer& b     = core.Sound();
    int           count = 0;
    for(const Grain& g : core.Grains())
    {
        if(!g.IsActive() || g.Source() != &b || b.Length() == 0)
            continue;
        double position = g.Position() / double(b.Length());
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
    params[kDensity]      = p.density;
    params[kGrainMs]      = p.grain_ms;
    params[kPitch]        = 12.0f * std::log2(p.pitch);
    params[kReverb]       = p.reverb;
    params[kJitter]       = p.jitter;
    params[kRandPitch]    = p.rand_pitch;
    params[kWindowShape]  = p.window_shape;
    params[kStart]        = p.start;
    params[kLength]       = p.length;
    params[kMix]          = p.mix;
    params[kFilter]       = p.filter;
    params[kRandPos]      = p.rand_pos;
    params[kRandSize]     = p.rand_size;
    params[kRandShape]    = p.rand_shape;
    return params;
}
