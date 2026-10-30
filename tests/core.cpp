// Host tests: the core and the Simple Touch mapping on a computer, no board.
// Built and run by tests/run.sh (make test). Each check prints PASS or FAIL.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>
#include "core/svf.h"
#include "ui/core.ui.h"

using namespace clessidra;

namespace
{
constexpr size_t kRate    = 48000;
constexpr size_t kBlock   = 48;
int              failures = 0;

void Check(bool ok, const char* name, double value)
{
    std::printf("%s  %-52s %.4g\n", ok ? "PASS" : "FAIL", name, value);
    if(!ok)
        failures++;
}

bool Near(double value, double target, double tolerance)
{
    return std::fabs(value - target) <= tolerance;
}

// A core with the board's memory: an 8 s recording, a 9 s ring
struct Rig
{
    std::vector<float> ring = std::vector<float>(9 * kRate);
    std::vector<float> memory[Core::kNumBuffers]
        = {std::vector<float>(8 * kRate), std::vector<float>(8 * kRate)};
    Core   core;
    double time   = 0.0; // seconds of input so far
    float  peak   = 0.0f;
    bool   finite = true;

    Rig()
    {
        float* buffers[Core::kNumBuffers]
            = {memory[0].data(), memory[1].data()};
        core.Init(kRate, ring.data(), ring.size(), buffers, 8 * kRate);
    }

    // Runs the input for seconds; between blocks, each(block)
    void Run(double                              seconds,
             const std::function<float(double)>& input,
             const std::function<void(size_t)>&  each = nullptr)
    {
        float        l[kBlock], o1[kBlock], o2[kBlock];
        const float* in[2]  = {l, l};
        float*       out[2] = {o1, o2};
        const size_t blocks = size_t(seconds * kRate / kBlock);
        for(size_t b = 0; b < blocks; b++)
        {
            if(each)
                each(b);
            for(size_t i = 0; i < kBlock; i++)
            {
                l[i] = input(time);
                time += 1.0 / kRate;
            }
            core.Process(in, out, kBlock);
            for(size_t i = 0; i < kBlock; i++)
            {
                finite = finite && std::isfinite(o1[i]) && std::isfinite(o2[i]);
                peak   = std::max(peak,
                                std::max(std::fabs(o1[i]), std::fabs(o2[i])));
            }
        }
    }

    void Hold(double seconds, const std::function<float(double)>& input)
    {
        core.SetRecording(true);
        Run(seconds, input);
        core.SetRecording(false);
        Run(0.6, input); // the copy lands
    }

    double Seconds() const { return core.RecordedSeconds(); }
};

float Sine(double t)
{
    return 0.5f * float(std::sin(2.0 * M_PI * 220.0 * t));
}
float Silence(double)
{
    return 0.0f;
}

void Capture()
{
    Rig r;
    // 2 s silence, then sound: a tap keeps the 8 s before it, without the
    // silence
    const double begin = 2.0;
    auto later = [begin](double t) { return t < begin ? 0.0f : Sine(t); };
    r.Run(4.0, later);
    r.Hold(0.1, later);
    Check(Near(r.Seconds(), 2.0, 0.03),
          "tap: the sound before it, silence cut (s)",
          r.Seconds());

    r.Run(1.0, Silence); // longer than the pre-roll
    r.Hold(1.0, Silence);
    Check(Near(r.Seconds(), 2.0, 0.03),
          "hold over silence: recording unchanged (s)",
          r.Seconds());

    r.Hold(15.0, Sine);
    Check(Near(r.Seconds(), 8.0, 0.001),
          "hold 15 s: the last 8 s (s)",
          r.Seconds());

    r.Hold(0.1, Sine);
    r.Hold(0.1, Sine);
    Check(Near(r.Seconds(), 8.0, 0.001),
          "two taps close together: 8 s each (s)",
          r.Seconds());

    // Trim: 5 ms kept before the first sound and after the last
    Rig  t;
    auto burst = [](double x) { return x >= 0.5 && x < 1.5 ? Sine(x) : 0.0f; };
    t.Hold(3.0, burst);
    const Buffer& rec   = t.core.Recording();
    size_t        first = rec.Length(), last = 0;
    for(size_t i = 0; i < rec.Length(); i++)
    {
        if(std::fabs(rec.Data()[i]) > Ring::kSilence)
        {
            if(first == rec.Length())
                first = i;
            last = i;
        }
    }
    const double margin = Buffer::kFadeSeconds * kRate;
    Check(Near(double(first), margin, 1.0),
          "trim: samples kept before the sound",
          double(first));
    Check(Near(double(rec.Length() - 1 - last), margin, 1.0),
          "trim: samples kept after the sound",
          double(rec.Length() - 1 - last));
    Check(t.finite, "output finite throughout", 1.0);
}

void Level()
{
    // Noise recorded, then played at 8 grains at once: the normaliser keeps the
    // input's level
    Rig      r;
    uint32_t seed  = 1;
    auto     noise = [&seed](double) {
        seed = seed * 1664525u + 1013904223u;
        return 0.3f * (float(seed >> 8) / float(1 << 24) * 2.0f - 1.0f);
    };
    r.Hold(2.0, noise);
    Core::Params p;
    p.mix     = 1.0f;
    p.density = 80.0f;
    r.core.SetParams(p);
    double       in = 0.0, out = 0.0;
    float        l[kBlock], o1[kBlock], o2[kBlock];
    const float* ins[2]  = {l, l};
    float*       outs[2] = {o1, o2};
    for(size_t b = 0; b < 4000; b++)
    {
        for(size_t i = 0; i < kBlock; i++)
            l[i] = noise(0.0);
        r.core.Process(ins, outs, kBlock);
        if(b < 1000)
            continue;
        for(size_t i = 0; i < kBlock; i++)
        {
            in += double(l[i]) * l[i];
            out += 0.5 * (double(o1[i]) * o1[i] + double(o2[i]) * o2[i]);
        }
    }
    const double db = 10.0 * std::log10(out / in);
    Check(
        db > 4.0 && db < 6.5, "8 grains at once: 6 dB over the input (dB)", db);

    // Heavy: 20 ms grains, the most density, reverb: never above the limiter
    p.grain_ms   = 20.0f;
    p.density    = Core::kMaxDensity;
    p.reverb     = 0.8f;
    p.rand_pitch = 0.5f;
    r.core.SetParams(p);
    r.peak = 0.0f;
    r.Run(3.0, Sine);
    Check(r.peak <= Limiter::kThreshold + 1e-4f,
          "heavy: output peak at most -0.5 dBFS",
          r.peak);
}

// One grain of 1000 samples over a recording of ones: its window, sample by
// sample
std::vector<float> Window(float shape)
{
    std::vector<float> ones(kRate, 1.0f);
    Buffer             buffer;
    buffer.Init(ones.data(), ones.size(), float(kRate));
    std::fill(ones.begin(), ones.end(), 1.0f);
    buffer.Finish(ones.size());
    Grain::InitWindow();
    Grain        grain;
    Grain::Latch latch;
    latch.buffer   = &buffer;
    latch.position = kRate / 2;
    latch.rate     = 0.0f;
    latch.pan      = 0.0f; // all left, gain 1
    latch.shape    = shape;
    grain.Start(latch, 0);
    std::vector<float> window(1000, 0.0f), right(1000, 0.0f);
    Grain::Sums        sums;
    grain.Render(
        1.0 / 1000.0, window.data(), right.data(), window.size(), sums);
    return window;
}

void Windows()
{
    const auto hann = Window(0.5f);
    Check(Near(hann[250], 0.5, 0.01) && Near(hann[500], 1.0, 0.01),
          "window 0.5: Hann (at 1/4, 1/2)",
          hann[250]);
    const auto   percussive = Window(1.0f);
    const size_t peak
        = size_t(std::max_element(percussive.begin(), percussive.end())
                 - percussive.begin());
    Check(peak == 10 && Near(percussive[500], 0.060, 0.005),
          "window 1: percussive (peak at 1 %, half way)",
          percussive[500]);
    const auto reverse = Window(0.0f);
    Check(Near(reverse[999 - 500], percussive[500], 0.002),
          "window 0: percussive reversed",
          reverse[499]);
}

// Share of the output's power within 10 Hz of the partials of 55 Hz, from 40 Hz
// to 1 kHz
double Gain(float hz, float cutoff, float q, bool low)
{
    Svf f;
    f.Set(cutoff, q, float(kRate));
    double in = 0.0, out = 0.0;
    for(size_t n = 0; n < 2 * kRate; n++)
    {
        const float x = float(std::sin(2.0 * M_PI * hz * double(n) / kRate));
        f.Process(x);
        const float y = low ? f.Low() : f.High();
        if(n < kRate)
            continue;
        in += double(x) * x;
        out += double(y) * y;
    }
    return 10.0 * std::log10(out / in);
}

void Tone()
{
    Check(Near(Gain(4000, 200, 1.5f, true), -52.0, 1.0),
          "tone: lowpass 200 Hz at 4 kHz (dB)",
          Gain(4000, 200, 1.5f, true));
    Check(Near(Gain(200, 2000, 1.5f, false), -40.0, 1.0),
          "tone: highpass 2 kHz at 200 Hz (dB)",
          Gain(200, 2000, 1.5f, false));
    Check(std::fabs(Gain(10000, 20000, 0.707f, true)) < 0.05,
          "tone: off (lowpass 20 kHz) at 10 kHz (dB)",
          Gain(10000, 20000, 0.707f, true));
}

void Ends()
{
    Playhead one_shot;
    double   phase = 0.0;
    for(int i = 0; i < 20; i++)
        phase
            = one_shot.Step(phase, 0.1, Playhead::End::OneShot, 0.0, 1.0, 0.0);
    Check(one_shot.IsStopped() && phase < 1.0,
          "one-shot: stops at the end",
          phase);
    const double stopped = phase;
    phase = one_shot.Step(phase, 0.1, Playhead::End::OneShot, 0.0, 1.0, 0.0);
    Check(phase == stopped, "one-shot: stays stopped", phase);
    phase = one_shot.Step(phase, -0.1, Playhead::End::OneShot, 0.0, 1.0, 0.0);
    Check(!one_shot.IsStopped() && phase < stopped,
          "one-shot: flipped, it falls back",
          phase);

    Playhead ping_pong;
    phase        = 0.5;
    double least = 1.0;
    double most  = 0.0;
    bool   back  = false;
    for(int i = 0; i < 100; i++)
    {
        phase = ping_pong.Step(
            phase, 0.03, Playhead::End::PingPong, 0.0, 1.0, 0.1);
        least = std::min(least, phase);
        most  = std::max(most, phase);
        back  = back || ping_pong.IsBackward(0.03);
    }
    Check(back && most < 0.9 && least >= 0.1 - 0.03,
          "ping-pong: turns inside the margins (highest phase)",
          most);

    Playhead loop;
    Check(Near(loop.Step(0.95, 0.1, Playhead::End::Loop, 0.0, 1.0, 0.0),
               0.05,
               1e-9),
          "loop: wraps",
          0.05);

    Rig r;
    r.Hold(1.0, Sine);
    Core::Params p;
    p.end = Playhead::End::OneShot;
    r.core.SetParams(p);
    auto sounding = [&r] {
        int n = 0;
        for(const auto& grain : r.core.Grains())
            n += grain.IsActive();
        return n;
    };
    r.Run(2.0, Silence);
    Check(sounding() == 0,
          "one-shot: silence after the end (grains)",
          sounding());
    r.core.Restart();
    r.Run(0.05, Silence);
    Check(
        sounding() > 0, "one-shot: restart sounds again (grains)", sounding());

    // Every slice lasts the same
    auto duration = [&r, &p, &sounding](float start) {
        p.slice_start = start;
        p.slice_end   = start + 1.0f / 7.0f;
        r.core.SetParams(p);
        r.core.Restart();
        double seconds = 0.0;
        for(int i = 0; i < 300 && (i < 2 || sounding() > 0); i++)
        {
            r.Run(0.01, Silence);
            seconds += 0.01;
        }
        return seconds;
    };
    const double first = duration(0.0f);
    const double last  = duration(6.0f / 7.0f);
    Check(Near(first, last, 0.02),
          "one-shot: first and last slice last the same (s)",
          last - first);
}

// L/R correlation of grains from different parts of the sound
double Width(float density, float spread)
{
    Rig r;
    r.Hold(2.0, Sine);
    Core::Params p;
    p.density  = density;
    p.rand_pos = 1.0f;
    p.spread   = spread;
    p.mix      = 1.0f;
    r.core.SetParams(p);
    r.Run(0.5, Silence);
    float        in[kBlock] = {}, l[kBlock], o[kBlock];
    const float* inputs[2]  = {in, in};
    float*       outputs[2] = {l, o};
    double       ll = 0.0, rr = 0.0, lr = 0.0;
    for(size_t b = 0; b < size_t(2 * kRate / kBlock); b++)
    {
        r.core.Process(inputs, outputs, kBlock);
        for(size_t i = 0; i < kBlock; i++)
        {
            ll += l[i] * l[i];
            rr += o[i] * o[i];
            lr += l[i] * o[i];
        }
    }
    return lr / std::sqrt(ll * rr);
}

void Pan()
{
    Check(Width(130.0f, 0.0f) > 0.999,
          "spread 0: grains at the centre (L/R correlation)",
          Width(130.0f, 0.0f));
    const double lone = Width(10.0f, 1.0f);
    const double full = Width(130.0f, 1.0f);
    Check(full < 0.8 && lone > full + 0.1,
          "spread 1: wider with more grains (L/R correlation)",
          full);
}

void Voices()
{
    Rig r;
    r.Hold(1.0, Sine);
    Core::Params p;
    p.density  = Core::kMaxDensity;
    p.grain_ms = 4000.0f;
    p.jitter   = 1.0f;
    r.core.SetParams(p);
    size_t most = 0;
    double sum  = 0.0;
    r.Run(3.0, Silence, [&](size_t) {
        size_t n = 0;
        for(const auto& grain : r.core.Grains())
            n += grain.IsActive();
        most = std::max(most, n);
        sum += r.core.GrainLevel();
    });
    Check(most < Core::kNumVoices,
          "the most density, long grains: a voice stays free (most sounding)",
          double(most));
    Check(sum / (3.0 * kRate / kBlock) > 10.0,
          "the most density, long grains: whole windows (mean of the windows)",
          sum / (3.0 * kRate / kBlock));
    Check(r.finite, "the most density, long grains: output finite", 1.0);
}

void Onsets()
{
    // Clicks on silence: an edge at each, within 10 ms, first to last
    std::vector<float>          clicks(4 * kRate, 0.0f);
    const std::array<double, 6> times = {0.5, 1.0, 1.5, 2.0, 2.5, 3.0};
    for(double t : times)
    {
        for(size_t i = 0; i < 480; i++)
            clicks[size_t(t * kRate) + i] = 0.5f * (i % 2 ? 1.0f : -1.0f);
    }
    const auto edges = FindSlices(clicks.data(), clicks.size(), kRate);
    bool       found = edges[0] == 0.0f && edges[kNumSlices] == 1.0f;
    for(size_t i = 0; i < times.size(); i++)
        found = found && Near(edges[i + 1] * 4.0, times[i], 0.01);
    Check(found,
          "onsets: an edge at each click (second edge, s)",
          edges[1] * 4.0);

    // A sustained sound: slices all the same
    std::vector<float> sine(4 * kRate);
    for(size_t i = 0; i < sine.size(); i++)
        sine[i] = Sine(double(i) / kRate);
    const auto even   = FindSlices(sine.data(), sine.size(), kRate);
    bool       rising = true;
    for(size_t i = 0; i < kNumSlices; i++)
        rising = rising && even[i + 1] > even[i];
    Check(rising && even[kNumSlices] == 1.0f,
          "onsets: none found, the slices still rise to the end",
          even[1]);
}

void Mapping()
{
    Rig      r;
    CoreUI   ui;
    Controls c;
    c.knobs.fill(0.5f);
    c.knobs[Controls::S30] = 0.0f;
    c.knobs[Controls::S32] = 0.0f;
    c.knobs[Controls::S35] = 1.0f;
    c.knobs[Controls::S36] = 0.75f;
    c.knobs[Controls::S37] = 0.0f;
    c.switches             = {Controls::Up, Controls::Up};
    ui.Process(c, r.core);
    const Core::Params& p = r.core.CurrentParams();
    Check(Near(p.grain_ms, 100.0, 0.01),
          "S31 centre: grain size (ms)",
          p.grain_ms);
    Check(Near(p.pitch, 1.0, 1e-6), "S33 centre: pitch (ratio)", p.pitch);
    Check(Near(p.speed, 1.0, 1e-6) && !p.freeze,
          "S36 half way up: speed 1",
          p.speed);
    Check(p.density == 0.0f, "S37 bottom: density off", p.density);
    Check(Near(p.tone, 0.0, 1e-6), "P11 + S34 default: tone off", p.tone);
    Check(p.jitter == 0.0f && p.rand_pos == 0.0f && p.rand_pitch == 0.0f
              && p.spread == 0.0f,
          "S32 bottom: plain",
          p.jitter);
    Check(p.end == Playhead::End::OneShot, "S07 up: one-shot", int(p.end));

    c.knobs[Controls::S36] = 0.25f;
    ui.Process(c, r.core);
    Check(Near(p.speed, -1.0, 1e-6), "S36 half way down: speed -1", p.speed);
    c.knobs[Controls::S36] = 0.5f;
    ui.Process(c, r.core);
    Check(p.freeze, "S36 centre: stop", p.speed);
    c.knobs[Controls::S36] = 0.75f;

    c.knobs[Controls::S33] = 0.5f + 2.0f / 14.0f; // on the 1 semitone stop
    ui.Process(c, r.core);
    Check(Near(p.pitch, std::exp2(1.0 / 12.0), 1e-3),
          "S33: a stop at 1 semitone",
          p.pitch);
    c.knobs[Controls::S33] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.pitch, 4.0, 1e-3), "S33 top: +2 octaves", p.pitch);
    c.knobs[Controls::S33] = 0.5f;

    c.knobs[Controls::S32] = 0.5f;
    ui.Process(c, r.core);
    Check(p.jitter == 0.5f && p.rand_pitch == 0.0f,
          "S32 centre: half random, pitch still plain",
          p.jitter);
    c.knobs[Controls::S32] = 1.0f;
    ui.Process(c, r.core);
    Check(p.rand_pitch == 1.0f, "S32 top: random pitch", p.rand_pitch);
    c.knobs[Controls::S32] = 0.0f;

    c.switches[Controls::S07] = Controls::Down;
    ui.Process(c, r.core);
    Check(p.end == Playhead::End::Loop, "S07 down: loop", int(p.end));

    // P10 held through the UI records; the LED glows with the grains
    c.knobs[Controls::S37] = 0.8f;
    uint32_t ms            = 0;
    auto     press         = [&](size_t pad, bool on) {
        c.touched[pad] = on;
        ui.Process(c, r.core);
    };
    r.Run(1.5, Sine, [&](size_t b) {
        c.now_ms = ms = uint32_t(b);
        press(CoreUI::kRecordPad, b >= 500 && b < 1000); // 1 ms blocks
    });
    Check(Near(r.Seconds(), 0.55, 0.01),
          "P10 held 0.5 s: recording, with pre-roll (s)",
          r.Seconds());
    r.Run(0.5, Sine);
    const float led = ui.Led(ms + 1000, r.core);
    Check(Near(led, 0.6 * 0.6 * 0.6, 0.01), "LED: dim glow with grains", led);

    // P11 held and S34 turned: the tone changes, the shape doesn't
    const float shape_before = p.window_shape;
    const float tone_before  = ui.Effect(CoreUI::kToneKnob);
    press(CoreUI::kEffectsPad, true);
    for(int i = 1; i <= 20; i++)
    {
        c.knobs[Controls::S34] = 0.5f + 0.02f * float(i);
        ui.Process(c, r.core);
    }
    press(CoreUI::kEffectsPad, false);
    Check(ui.Effect(CoreUI::kToneKnob) > tone_before + 0.2f,
          "P11 + S34: tone turned up",
          ui.Effect(CoreUI::kToneKnob));
    Check(p.window_shape == shape_before,
          "P11 + S34: shape unchanged",
          p.window_shape);
    c.knobs[Controls::S34] = 0.5f;
    ui.Process(c, r.core);

    // Grain size ends; a slice pressed firmly
    c.knobs[Controls::S31] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, 10.0, 0.01),
          "S31 bottom: grain size (ms)",
          p.grain_ms);
    c.knobs[Controls::S31] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, 500.0 * r.Seconds(), 0.1),
          "S31 top: half the recording (ms)",
          p.grain_ms);
    const float  density = p.density;
    const size_t place   = CoreUI::kSlicePads[3];
    c.pressure[place]    = 0.3f;
    press(place, true);
    Check(p.density == density,
          "slice touched lightly: density unchanged",
          p.density);
    r.Run(0.001, Silence);
    Check(Near(r.core.PlayheadPhase(), p.slice_start, 0.01),
          "slice touched: the playhead moves to its start (phase)",
          r.core.PlayheadPhase());
    c.pressure[place] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.density, Core::kMaxDensity, 0.01),
          "slice pressed hard: the most density",
          p.density);
    press(place, false);
    Check(p.slice_end == 1.0f,
          "released, in loop: the whole selection",
          p.slice_end);

    // P11 held and S37 down: output down; the main layer's density unchanged
    const float density_before = p.density;
    press(CoreUI::kEffectsPad, true);
    for(int i = 1; i <= 50; i++)
    {
        c.knobs[Controls::S37] = 1.0f - 0.02f * float(i);
        ui.Process(c, r.core);
    }
    const float output = p.output;
    press(CoreUI::kEffectsPad, false);
    Check(output < 0.01f && Near(p.density, density_before, 1e-4),
          "P11 + S37 down: output off, density kept",
          output);

    // Detent pulse: on reaching the centre, renewed while turned within it
    c.knobs[Controls::S33] = 0.4f;
    c.now_ms               = 10000;
    ui.Process(c, r.core);
    c.knobs[Controls::S33] = 0.49f;
    ui.Process(c, r.core);
    const float reached    = ui.Led(10000 + CoreUI::kPulseMs - 1, r.core);
    c.now_ms               = 10200;
    c.knobs[Controls::S33] = 0.51f;
    ui.Process(c, r.core);
    const float renewed = ui.Led(10200 + CoreUI::kPulseMs - 1, r.core);
    const float ended   = ui.Led(10200 + CoreUI::kPulseMs, r.core);
    Check(reached == 1.0f && renewed == 1.0f && ended < 1.0f,
          "LED: pulse at pitch unison, renewed while turned",
          renewed);
}

} // namespace

int main()
{
    Capture();
    Level();
    Windows();
    Tone();
    Ends();
    Pan();
    Voices();
    Onsets();
    Mapping();
    std::printf("%s\n",
                failures ? "Some checks failed." : "All checks passed.");
    return failures ? 1 : 0;
}
