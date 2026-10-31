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

// A core with a buffer of the given length
struct Rig
{
    std::vector<float> memory;
    Core               core;
    double             time   = 0.0; // seconds of input so far
    float              peak   = 0.0f;
    bool               finite = true;

    explicit Rig(double seconds = 8.0) : memory(size_t(seconds * kRate))
    {
        core.Init(kRate, memory.data(), memory.size());
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

    // The whole buffer filled with the input, then sealed
    void Fill(const std::function<float(double)>& input)
    {
        Core::Params p;
        p.fill = 1.0f;
        core.SetParams(p);
        Run(double(memory.size()) / kRate + 0.1, input);
        core.SetParams(Core::Params{});
    }
};

float Sine(double t)
{
    return 0.5f * float(std::sin(2.0 * M_PI * 220.0 * t));
}
float Silence(double)
{
    return 0.0f;
}

double Rms(const std::vector<float>& x)
{
    double sum = 0.0;
    for(float v : x)
        sum += double(v) * v;
    return std::sqrt(sum / double(x.size()));
}

void FillTests()
{
    Rig empty(1.0);
    empty.Run(0.5, Sine);
    int grains = 0;
    for(const auto& grain : empty.core.Grains())
        grains += grain.IsActive();
    Check(grains == 0, "nothing in the buffer: no grains", grains);

    // The first sound is heard at once, whatever the end and the selection
    for(auto end :
        {Playhead::End::OneShot, Playhead::End::PingPong, Playhead::End::Loop})
    {
        for(float position : {0.0f, 0.5f})
        {
            Rig          first(8.0);
            Core::Params q;
            q.end    = end;
            q.fill   = 1.0f;
            q.start  = position;
            q.length = 0.25f;
            first.core.SetParams(q);
            first.Run(0.3, Sine);
            Check(first.peak > 0.2f,
                  "first sound: heard within 0.3 s",
                  first.peak);
        }
    }

    // Live at speed 1: the distance to the input stays, S30 sets it
    {
        Rig          live(8.0);
        Core::Params q;
        q.fill     = 1.0f;
        q.grain_ms = 50.0f;
        live.core.SetParams(q);
        live.Run(9.0, Sine);
        const double phase = live.core.LivePhase();
        live.Run(1.0, Sine);
        Check(live.core.LivePhase() == phase && phase > 0.99,
              "live, speed 1: the playhead stays at the newest sound",
              live.core.LivePhase());
    }

    Rig r(1.0);
    r.Fill(Sine);
    const double full = Rms(r.memory);
    Check(Near(full, 0.5, 0.01),
          "fill 1: the input, two jacks +3 dB (rms)",
          full);

    r.Run(0.01, Silence);
    const double sealed = Rms(r.memory);
    r.Run(1.0, Silence);
    Check(Rms(r.memory) == sealed && Near(sealed, full, 0.001),
          "fill 0: sealed, nothing changes",
          Rms(r.memory));

    Core::Params p;
    p.fill = 0.5f;
    r.core.SetParams(p);
    r.Run(1.0, Silence);
    Check(Near(Rms(r.memory), 0.5 * full, 0.03 * full),
          "fill 0.5: half the old sound after one pass (rms)",
          Rms(r.memory));

    // Filled for a part of the buffer: the sound is that part
    Rig part(8.0);
    p      = Core::Params{};
    p.fill = 1.0f;
    part.core.SetParams(p);
    part.Run(2.0, Sine);
    part.core.SetParams(Core::Params{});
    part.Run(0.1, Silence);
    Check(Near(double(part.core.SoundSamples()), 2.0 * kRate, 0.11 * kRate),
          "filled for 2 s, then sealed: 2 s of sound (samples)",
          double(part.core.SoundSamples()));
    int          silent = 0;
    Core::Params play;
    play.density = 50.0f;
    part.core.SetParams(play);
    part.Run(4.0, Silence, [&](size_t) {
        if(part.core.GrainLevel() > 0.1f
           && part.core.PlayheadPhase() * 2.05 * kRate > 2.2 * kRate)
            silent++;
    });
    Check(silent == 0, "sealed: the playhead stays in the sound", silent);

    p = Core::Params{};
    // Live, reading across the write point: no build-up, nothing unsafe
    p.fill     = 1.0f;
    p.pitch    = 2.0f;
    p.rand_pos = 1.0f;
    r.core.SetParams(p);
    r.peak = 0.0f;
    r.Run(3.0, Sine);
    Check(r.finite && r.peak <= Limiter::kThreshold + 1e-4f,
          "live, pitched up: output stays below the limiter",
          r.peak);
}

void Level()
{
    // Noise in the buffer, then played at 8 grains at once: the normaliser
    // keeps the
    // input's level
    Rig      r(2.0);
    uint32_t seed  = 1;
    auto     noise = [&seed](double) {
        seed = seed * 1664525u + 1013904223u;
        return 0.3f * (float(seed >> 8) / float(1 << 24) * 2.0f - 1.0f);
    };
    r.Fill(noise);
    Core::Params p;
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
    Check(db > 6.0 && db < 9.5,
          "8 grains at once: 9 dB over two jacks of input, at most (dB)",
          db);

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

// The whole panel on the core, as on the board
struct Panel
{
    Rig      rig;
    CoreUI   ui;
    Controls c;
    uint32_t ms = 0;

    explicit Panel(double seconds = 8.0) : rig(seconds)
    {
        c.knobs.fill(0.5f);
        c.knobs[Controls::S30] = 0.0f;
        c.knobs[Controls::S32] = 0.0f;
        c.knobs[Controls::S34] = 0.0f;
        c.knobs[Controls::S35] = 1.0f;
        c.knobs[Controls::S36] = 0.75f;
        c.knobs[Controls::S37] = 0.75f;
        c.switches             = {Controls::Up, Controls::Centre};
    }

    void Run(double seconds, const std::function<float(double)>& input)
    {
        rig.Run(seconds, input, [this](size_t) {
            c.now_ms = ms++;
            ui.Process(c, rig.core);
        });
    }

    // Seconds until the output stays under -60 dB with no input
    double Fade()
    {
        double       last = 0.0, now = 0.0;
        float        l[kBlock] = {}, o1[kBlock], o2[kBlock];
        const float* in[2]     = {l, l};
        float*       out[2]    = {o1, o2};
        for(size_t b = 0; b < size_t(14.0 * kRate / kBlock); b++)
        {
            c.now_ms = ms++;
            ui.Process(c, rig.core);
            rig.core.Process(in, out, kBlock);
            now += double(kBlock) / kRate;
            for(size_t i = 0; i < kBlock; i++)
            {
                if(std::fabs(o1[i]) > 0.001f || std::fabs(o2[i]) > 0.001f)
                    last = now;
            }
        }
        return last;
    }
};

void Live()
{
    // Fill up: whatever the panel, the sound follows the input, and is gone
    // soon after it. The reverb sets how soon.
    struct Case
    {
        const char* name;
        float       s30, s31, s32, s35, s36, s37;
        double      most;
    };
    const Case cases[] = {
        {"pour low, reverb", 0.0f, 0.5f, 0.0f, 1.0f, 0.75f, 0.25f, 3.5},
        {"pour middle", 0.0f, 0.5f, 0.0f, 1.0f, 0.75f, 0.5f, 3.0},
        {"pour 75 %", 0.0f, 0.5f, 0.0f, 1.0f, 0.75f, 0.75f, 2.0},
        {"pour top", 0.0f, 0.5f, 0.0f, 1.0f, 0.75f, 1.0f, 2.0},
        {"position up", 0.8f, 0.5f, 0.0f, 1.0f, 0.75f, 0.75f, 2.0},
        {"short selection", 0.5f, 0.5f, 0.0f, 0.1f, 0.75f, 0.75f, 2.0},
        {"scatter a little", 0.0f, 0.5f, 0.25f, 1.0f, 0.75f, 0.75f, 2.0},
        {"long grains", 0.0f, 1.0f, 0.0f, 1.0f, 0.75f, 0.75f, 3.0},
    };
    for(const Case& k : cases)
    {
        Panel panel;
        panel.c.knobs[Controls::S30] = k.s30;
        panel.c.knobs[Controls::S31] = k.s31;
        panel.c.knobs[Controls::S32] = k.s32;
        panel.c.knobs[Controls::S34] = 1.0f;
        panel.c.knobs[Controls::S35] = k.s35;
        panel.c.knobs[Controls::S36] = k.s36;
        panel.c.knobs[Controls::S37] = k.s37;
        panel.Run(10.0, Sine);
        const double fade = panel.Fade();
        std::printf("      fill 100 %%, %s: gone in %.2f s\n", k.name, fade);
        Check(fade < k.most, "live: gone soon after the input stops (s)", fade);
    }

    // Pitched up: grains don't run into the oldest sound
    Panel high;
    high.c.knobs[Controls::S33] = 1.0f;
    high.c.knobs[Controls::S34] = 1.0f;
    high.Run(10.0, Sine);
    const double high_fade = high.Fade();
    Check(high_fade < 2.5,
          "live, pitch +24: gone soon after the input (s)",
          high_fade);

    // Scatter at the top reaches 2 s back, no further
    Panel wide;
    wide.c.knobs[Controls::S32] = 1.0f;
    wide.c.knobs[Controls::S34] = 1.0f;
    wide.Run(10.0, Sine);
    const double wide_fade = wide.Fade();
    Check(wide_fade < 5.0,
          "live, scatter top: gone within its reach (s)",
          wide_fade);

    // Slowed down, then back at speed 1: back to what comes in
    Panel back;
    back.c.knobs[Controls::S34] = 1.0f;
    back.c.knobs[Controls::S36] = 0.6f;
    back.Run(9.0, Sine);
    back.c.knobs[Controls::S36] = 0.75f;
    back.Run(1.0, Sine);
    const double fade = back.Fade();
    Check(fade < 2.0,
          "live, back at speed 1: gone soon after the input (s)",
          fade);

    // Slowed down, it falls behind: it hears what passed, by design
    Panel slow;
    slow.c.knobs[Controls::S34] = 1.0f;
    slow.c.knobs[Controls::S36] = 0.6f;
    slow.Run(10.0, Sine);
    Check(
        slow.rig.peak > 0.2f, "live, slowed down: still sounds", slow.rig.peak);

    // Sealed: the sound stays while Pour is open, whatever the input does
    Panel sealed;
    sealed.c.knobs[Controls::S34]    = 1.0f;
    sealed.c.switches[Controls::S07] = Controls::Down;
    sealed.Run(4.0, Sine);
    sealed.c.knobs[Controls::S34] = 0.0f;
    sealed.Run(0.1, Silence);
    sealed.rig.peak = 0.0f;
    sealed.Run(5.0, Silence);
    Check(sealed.rig.peak > 0.2f, "sealed: the sound stays", sealed.rig.peak);
    Check(
        Near(double(sealed.rig.core.SoundSamples()), 4.1 * kRate, 0.2 * kRate),
        "sealed: as much sound as came in (samples)",
        double(sealed.rig.core.SoundSamples()));
}

// Output rms over seconds, under the panel's controls
double PanelRms(Panel& panel, double seconds, float (*input)(double))
{
    double       sum = 0.0;
    size_t       n   = 0;
    float        l[kBlock], o1[kBlock], o2[kBlock];
    const float* in[2]  = {l, l};
    float*       out[2] = {o1, o2};
    for(size_t b = 0; b < size_t(seconds * kRate / kBlock); b++)
    {
        panel.c.now_ms = panel.ms++;
        panel.ui.Process(panel.c, panel.rig.core);
        for(size_t i = 0; i < kBlock; i++)
            l[i] = input(double(i) / kRate);
        panel.rig.core.Process(in, out, kBlock);
        for(size_t i = 0; i < kBlock; i++)
            sum += 0.5 * (double(o1[i]) * o1[i] + double(o2[i]) * o2[i]);
        n += kBlock;
    }
    return std::sqrt(sum / double(n));
}

float Noise(double)
{
    static uint32_t seed = 1;
    seed                 = seed * 1664525u + 1013904223u;
    return 0.5f * (float(seed >> 8) / float(1 << 24) * 2.0f - 1.0f);
}

// 8 s: silence, then the sine for the last 2 s, sealed. The sine is what is held.
void HoldSine(Panel& panel)
{
    panel.c.knobs[Controls::S34] = 1.0f;
    panel.Run(6.0, Silence);
    panel.Run(2.0, Sine);
    panel.c.knobs[Controls::S34]    = 0.0f;
    panel.c.knobs[Controls::S35]    = 0.2f;
    panel.c.knobs[Controls::S32]    = 0.0f;
    panel.c.switches[Controls::S07] = Controls::Down;
    panel.Run(0.5, Silence);
}

// Fill is the odds of a grain following the input: no jump, and every control
// the same at any Fill
void Held()
{
    // The moment Fill came down from: S30 at 0 ends at it
    Panel hold;
    HoldSine(hold);
    const double sealed = PanelRms(hold, 1.0, Silence);
    Check(sealed > 0.1, "sealed: the moment just heard (rms)", sealed);

    // Low Fill: the write point crosses silence, the selection stays
    Panel low;
    HoldSine(low);
    low.c.knobs[Controls::S34] = 0.05f;
    const double worn          = PanelRms(low, 1.0, Silence);
    Check(worn > 0.75 * sealed,
          "fill 5 %: still the selection, no jump (rms)",
          worn);

    // More Fill, fewer grains from the held sound
    double last = sealed;
    bool   less = true;
    for(float fill : {0.25f, 0.5f, 0.75f, 1.0f})
    {
        Panel more;
        HoldSine(more);
        more.c.knobs[Controls::S34] = fill;
        more.Run(2.5, Silence); // the reverb of what was before
        const double now = PanelRms(more, 1.0, Silence);
        std::printf(
            "      fill %.0f %%, input silent: rms %.4f\n", 100.0 * fill, now);
        less = less && now < last;
        last = now;
    }
    Check(less && last < 0.1 * sealed,
          "fill up: the held sound gives way to the input (rms at 100 %)",
          last);

    // The crossfade on a real spread of sound: noise held, noise coming in
    for(float fill : {0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 0.97f, 1.0f})
    {
        Panel held, input;
        held.c.knobs[Controls::S34] = 1.0f;
        held.Run(8.5, Noise);
        held.c.knobs[Controls::S34]    = 0.0f;
        held.c.switches[Controls::S07] = Controls::Down;
        held.Run(0.2, Silence);
        held.c.knobs[Controls::S34] = fill;
        held.Run(2.5, Silence);
        const double from_held       = PanelRms(held, 1.0, Silence);
        input.c.knobs[Controls::S34] = 1.0f;
        input.Run(8.5, Silence);
        input.c.knobs[Controls::S34]    = 0.0f;
        input.c.switches[Controls::S07] = Controls::Down;
        input.Run(0.2, Silence);
        input.c.knobs[Controls::S34] = fill;
        input.Run(2.5, Noise);
        const double from_input = PanelRms(input, 1.0, Noise);
        std::printf("      fill %3.0f %%: held %6.1f dB, input %6.1f dB\n",
                    100.0 * fill,
                    20.0 * std::log10(std::max(from_held, 1e-6)),
                    20.0 * std::log10(std::max(from_input, 1e-6)));
    }

    // S30 moves the selection at any Fill: back from the sine, into silence
    Panel away;
    HoldSine(away);
    away.c.knobs[Controls::S30] = 0.6f;
    away.c.knobs[Controls::S34] = 0.05f;
    away.Run(1.0, Silence);
    const double moved = PanelRms(away, 1.0, Silence);
    Check(moved < 0.1 * sealed,
          "fill 5 %: S30 still moves the selection (rms)",
          moved);

    // S07 at any Fill: one-shot ends, only the followers go on
    Panel ends;
    HoldSine(ends);
    ends.c.switches[Controls::S07] = Controls::Up;
    ends.c.knobs[Controls::S34]    = 0.05f;
    ends.ui.Process(ends.c, ends.rig.core);
    ends.rig.core.Restart();
    ends.Run(4.0, Silence);
    Check(ends.rig.core.HasEnded(), "fill 5 %: one-shot still ends", 1.0);

    // A slice at the top of Fill: back to what comes in once released
    for(auto end : {Controls::Up, Controls::Centre, Controls::Down})
    {
        Panel        pad;
        const size_t slice            = CoreUI::kSlicePads[0];
        pad.c.knobs[Controls::S34]    = 1.0f;
        pad.c.switches[Controls::S07] = end;
        pad.Run(9.0, Sine);
        pad.c.touched[slice] = true;
        pad.Run(0.5, Sine);
        pad.c.touched[slice] = false;
        pad.Run(0.5, Sine);
        const double fade = pad.Fade();
        Check(fade < 2.0,
              "live, slice released: gone soon after the input (s)",
              fade);
    }

    // Filled for a part of the buffer: no silence at any position
    for(float position : {0.0f, 0.3f, 0.7f})
    {
        Panel part;
        part.c.knobs[Controls::S34] = 1.0f;
        part.Run(2.0, Sine);
        part.c.knobs[Controls::S34]    = 0.0f;
        part.c.knobs[Controls::S30]    = position;
        part.c.knobs[Controls::S37]    = 1.0f;
        part.c.switches[Controls::S07] = Controls::Down;
        part.Run(0.5, Silence);
        double least = 1.0;
        for(int i = 0; i < 40; i++)
            least = std::min(least, PanelRms(part, 0.1, Silence));
        Check(least > 0.05,
              "part of the buffer: never silent (lowest rms)",
              least);
    }
}

// Every control moved at random, for a long time: nothing breaks
void Fuzz()
{
    Panel    panel;
    uint32_t seed = 12345;
    auto     next = [&seed] {
        seed = seed * 1664525u + 1013904223u;
        return float(seed >> 8) / float(1 << 24);
    };
    size_t most = 0;
    for(int step = 0; step < 4000; step++)
    {
        const size_t knob = size_t(next() * float(Controls::kNumKnobs));
        // Often at an end or a centre, where the special cases are
        const float value = next();
        panel.c.knobs[knob]
            = value < 0.2f ? 0.0f
                           : value < 0.4f ? 1.0f : value < 0.5f ? 0.5f : next();
        if(next() < 0.1f)
            panel.c.switches[Controls::S07]
                = Controls::Position(int(next() * 3.0f) % 3);
        const size_t pad      = CoreUI::kSlicePads[size_t(next() * 7.0f) % 7];
        panel.c.touched[pad]  = next() < 0.3f;
        panel.c.pressure[pad] = next();
        panel.Run(0.05, Sine);
        size_t n = 0;
        for(const auto& grain : panel.rig.core.Grains())
            n += grain.IsActive();
        most = std::max(most, n);
    }
    Check(panel.rig.finite, "every control at random: output finite", 1.0);
    Check(panel.rig.peak <= Limiter::kThreshold + 1e-4f,
          "every control at random: below the limiter",
          panel.rig.peak);
    Check(most < Core::kNumVoices,
          "every control at random: a voice stays free",
          double(most));
}

// One grain of 1000 samples over a buffer of ones: its window, sample by
// sample
std::vector<float> Window(float shape)
{
    std::vector<float> ones(kRate, 1.0f);
    Buffer             buffer;
    buffer.Init(ones.data(), ones.size());
    std::fill(ones.begin(), ones.end(), 1.0f);
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

    Rig r(1.0);
    r.Fill(Sine);
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
    Rig r(2.0);
    r.Fill(Sine);
    Core::Params p;
    p.density  = density;
    p.rand_pos = 1.0f;
    p.spread   = spread;
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
    Rig r(1.0);
    r.Fill(Sine);
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
    Rig      r(2.0);
    CoreUI   ui;
    Controls c;
    r.Fill(Sine);
    c.knobs.fill(0.5f);
    c.knobs[Controls::S30] = 0.0f;
    c.knobs[Controls::S32] = 0.0f;
    c.knobs[Controls::S34] = 0.0f;
    c.knobs[Controls::S35] = 1.0f;
    c.knobs[Controls::S36] = 0.75f;
    c.knobs[Controls::S37] = 0.0f;
    c.switches             = {Controls::Up, Controls::Up};
    ui.Process(c, r.core);
    const Core::Params& p = r.core.CurrentParams();
    Check(p.density == 0.0f, "S37 bottom: closed", p.density);
    Check(Near(p.pitch, 1.0, 1e-6), "S33 centre: pitch (ratio)", p.pitch);
    Check(Near(p.speed, 1.0, 1e-6) && !p.freeze,
          "S36 half way up: speed 1",
          p.speed);
    Check(p.jitter == 0.0f && p.rand_pos == 0.0f && p.rand_pitch == 0.0f
              && p.spread == 0.0f,
          "S32 bottom, low pour: plain",
          p.jitter);
    Check(p.fill == 0.0f, "S34 bottom: sealed", p.fill);
    c.knobs[Controls::S34] = 0.1f;
    ui.Process(c, r.core);
    Check(p.fill == 0.1f, "S34 at 10 %: fill 0.1", p.fill);
    c.knobs[Controls::S34] = 1.0f;
    ui.Process(c, r.core);
    Check(p.fill == 1.0f, "S34 top: only the input", p.fill);
    c.knobs[Controls::S34] = 0.0f;
    ui.Process(c, r.core);
    Check(p.end == Playhead::End::OneShot, "S07 up: one-shot", int(p.end));

    c.knobs[Controls::S37] = 0.1f;
    ui.Process(c, r.core);
    const float drop_ms = p.grain_ms, drop_reverb = p.reverb;
    Check(p.window_shape == 1.0f && p.density < 2.0f,
          "S37 low: rare percussive drops",
          p.density);
    c.knobs[Controls::S37] = 1.0f;
    ui.Process(c, r.core);
    Check(p.window_shape == 0.5f && p.grain_ms > 5.0f * drop_ms
              && p.reverb < drop_reverb && p.jitter > 0.25f && p.spread > 0.25f,
          "S37 top: long soft grains that differ, less reverb",
          p.grain_ms);
    Check(Near(p.density, Core::kMaxDensity, 0.01),
          "S37 top: the most density",
          p.density);
    c.knobs[Controls::S37] = 0.5f;
    ui.Process(c, r.core);
    const float centre_ms  = p.grain_ms;
    c.knobs[Controls::S31] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, 4.0 * centre_ms, 0.5),
          "S31 top: 2 octaves longer (ms)",
          p.grain_ms);
    c.knobs[Controls::S31] = 0.5f;

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

    // The LED glows with the grains
    c.knobs[Controls::S37] = 0.8f;
    uint32_t ms            = 0;
    auto     press         = [&](size_t pad, bool on) {
        c.touched[pad] = on;
        ui.Process(c, r.core);
    };
    ui.Process(c, r.core);
    r.Run(0.5, Silence);
    const float led = ui.Led(ms + 1000, r.core);
    Check(Near(led, 0.6 * 0.6 * 0.6, 0.01), "LED: dim glow with grains", led);

    // A slice: a light touch only starts it, a firm press opens Pour
    const float  density = p.density;
    const size_t place   = CoreUI::kSlicePads[3];
    c.pressure[place]    = 0.3f;
    press(place, true);
    Check(p.density == density,
          "slice touched lightly: pour unchanged",
          p.density);
    r.Run(0.001, Silence);
    Check(Near(r.core.PlayheadPhase(), p.slice_start, 0.01),
          "slice touched: the playhead moves to its start (phase)",
          r.core.PlayheadPhase());
    c.pressure[place] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.density, Core::kMaxDensity, 0.01),
          "slice pressed hard: pour wide open",
          p.density);
    press(place, false);
    Check(p.slice_end == 1.0f,
          "released, in loop: the whole selection",
          p.slice_end);

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
    FillTests();
    Live();
    Held();
    Fuzz();
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
