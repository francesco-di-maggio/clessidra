// Host tests: the core and the Simple Touch mapping on a computer, no board.
// Built and run by tests/run.sh (make test). Each check prints PASS or FAIL.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>
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

// A core with buffers of the given length, the ring a second longer
struct Rig
{
    std::vector<float> ring;
    std::vector<float> sound[Core::kNumBuffers];
    Core               core;
    double             time   = 0.0; // seconds of input so far
    float              peak   = 0.0f;
    bool               finite = true;

    explicit Rig(double seconds = 8.0) : ring(size_t((seconds + 1.0) * kRate))
    {
        float* buffers[Core::kNumBuffers];
        for(size_t i = 0; i < Core::kNumBuffers; i++)
        {
            sound[i].resize(size_t(seconds * kRate));
            buffers[i] = sound[i].data();
        }
        core.Init(kRate, ring.data(), ring.size(), buffers, sound[0].size());
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

    // Holds record for seconds of the input, then waits for it to land
    void Record(double seconds, const std::function<float(double)>& input)
    {
        const uint32_t before = core.Recordings();
        core.SetRecording(true);
        Run(seconds, input);
        core.SetRecording(false);
        for(int i = 0; i < 400 && core.Recordings() == before; i++)
            Run(0.01, input);
    }

    // The whole buffer recorded from the input
    void Fill(const std::function<float(double)>& input)
    {
        Record(double(sound[0].size()) / kRate, input);
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
        c.knobs[Controls::S35] = 1.0f;
        c.knobs[Controls::S32] = 0.0f;  // plain
        c.knobs[Controls::S37] = 0.75f; // random, about one grain at once
        c.switches = {Controls::Centre, Controls::Down}; // short reverb, loop
    }

    // P10 held for seconds of the input, then until it lands
    void Record(double seconds, const std::function<float(double)>& input)
    {
        const uint32_t before         = rig.core.Recordings();
        c.touched[CoreUI::kRecordPad] = true;
        Run(seconds, input);
        c.touched[CoreUI::kRecordPad] = false;
        for(int i = 0; i < 400 && rig.core.Recordings() == before; i++)
            Run(0.01, input);
    }

    void Run(double seconds, const std::function<float(double)>& input)
    {
        rig.Run(seconds, input, [this](size_t) {
            c.now_ms = ms++;
            ui.Process(c, rig.core);
        });
    }
};

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

double Seconds(const Core& core)
{
    return double(core.SoundSamples()) / kRate;
}

// P10: a tap keeps the last seconds, a hold the touch, both into the buffer
// not playing
void Record()
{
    Panel empty;
    empty.Run(1.0, Sine);
    Check(empty.rig.peak == 0.0f && Seconds(empty.rig.core) == 0.0,
          "power-up: silent until the first recording",
          empty.rig.peak);

    Panel hold;
    hold.Run(1.0, Sine);
    hold.Record(2.0, Sine);
    Check(Near(Seconds(hold.rig.core), 2.05, 0.02),
          "hold 2 s: the touch and 50 ms before (s)",
          Seconds(hold.rig.core));
    const double heard = PanelRms(hold, 0.5, Silence);
    Check(heard > 0.05, "hold: plays at once (rms)", heard);

    Panel tap;
    tap.Run(10.0, Sine);
    tap.Record(0.1, Sine);
    Check(Near(Seconds(tap.rig.core), Core::kTapCaptureSeconds, 0.02),
          "tap: the last 8 s (s)",
          Seconds(tap.rig.core));

    Panel shorter;
    shorter.rig.core.SetTapSeconds(4.0f);
    shorter.Run(10.0, Sine);
    shorter.Record(0.1, Sine);
    Check(Near(Seconds(shorter.rig.core), 4.0, 0.02),
          "tap, set to 4 s: the last 4 s (s)",
          Seconds(shorter.rig.core));

    Panel lng(4.0);
    lng.Record(6.0, Sine);
    Check(Near(Seconds(lng.rig.core), 4.0, 0.01),
          "held past the buffer: the last of it, all of the buffer (s)",
          Seconds(lng.rig.core));

    Panel quiet;
    quiet.Record(1.0, Sine);
    const double kept = Seconds(quiet.rig.core);
    quiet.Run(1.0, Silence);
    quiet.Record(1.0, Silence);
    Check(Seconds(quiet.rig.core) == kept,
          "silence recorded: nothing changes (s)",
          Seconds(quiet.rig.core));

    // Silence around the sound is trimmed to a fade. A tone with fades, as a
    // cut leaves the DC blocker's tail.
    Panel trim;
    trim.Run(1.0, Silence);
    trim.c.touched[CoreUI::kRecordPad] = true;
    trim.Run(0.5, Silence);
    const double from = trim.rig.time;
    trim.Run(1.0, [from](double t) {
        const double x = (t - from) / 1.0;
        return float(std::sin(M_PI * x)) * Sine(t);
    });
    trim.Run(0.5, Silence);
    trim.c.touched[CoreUI::kRecordPad] = false;
    trim.Run(2.0, Silence);
    Check(Near(Seconds(trim.rig.core), 1.0, 0.02),
          "silence at both ends: trimmed (s)",
          Seconds(trim.rig.core));

    // P10 held past a tap: the input heard with the grains. Otherwise only
    // the grains.
    auto  level = [](double) { return 0.25f; };
    Panel dry;
    dry.c.touched[CoreUI::kRecordPad] = true;
    dry.Run(0.5, level);
    const double through = PanelRms(dry, 0.3, level);
    Check(Near(through, 0.25 * std::sqrt(0.5), 0.002),
          "P10 held: the input heard, equal power (rms)",
          through);
    Panel wet;
    wet.Run(0.5, Sine);
    const double none = PanelRms(wet, 0.5, Sine);
    Check(none < 1e-6, "nothing recorded: no dry signal (rms)", none);

    // A new recording: the LED flashes while held, the playhead restarts
    Panel led;
    led.c.touched[CoreUI::kRecordPad] = true;
    led.Run(0.5, Sine);
    const float on  = led.ui.Led(0, led.rig.core);
    const float off = led.ui.Led(CoreUI::kFlashMs, led.rig.core);
    Check(on == 1.0f && off == 0.0f, "recording: the LED flashes", off);

    // Recording again while the last one plays: no jump at the swap
    Panel again;
    again.c.knobs[Controls::S37] = 1.0f;
    again.Record(2.0, Sine);
    float        l[kBlock], o1[kBlock], o2[kBlock];
    const float* in[2]                  = {l, l};
    float*       out[2]                 = {o1, o2};
    again.c.touched[CoreUI::kRecordPad] = true;
    for(size_t b = 0; b < size_t(4.0 * kRate / kBlock); b++)
    {
        if(b == size_t(1.0 * kRate / kBlock))
            again.c.touched[CoreUI::kRecordPad] = false;
        again.c.now_ms = again.ms++;
        again.ui.Process(again.c, again.rig.core);
        for(size_t i = 0; i < kBlock; i++)
            l[i] = Noise(0.0);
        again.rig.core.Process(in, out, kBlock);
        for(size_t i = 0; i < kBlock; i++)
            again.rig.finite = again.rig.finite && std::isfinite(o1[i])
                               && std::isfinite(o2[i]);
    }
    Check(again.rig.core.Recordings() == 2 && again.rig.finite,
          "recording while playing: lands, output finite",
          double(again.rig.core.Recordings()));
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
            panel.c.switches[size_t(next() * 2.0f) % 2]
                = Controls::Position(int(next() * 3.0f) % 3);
        for(size_t pad = 0; pad < Controls::kNumPads; pad++)
        {
            if(pad == CoreUI::kRecordPad)
                continue;
            panel.c.touched[pad]  = next() < 0.05f;
            panel.c.pressure[pad] = next();
        }
        if(next() < 0.05f)
            panel.c.touched[CoreUI::kRecordPad]
                = !panel.c.touched[CoreUI::kRecordPad];
        panel.Run(0.05, next() < 0.5f ? Sine : Noise);
        size_t n = 0;
        for(const auto& grain : panel.rig.core.Grains())
            n += grain.IsActive();
        most = std::max(most, n);
    }
    Check(panel.rig.finite, "every control at random: output finite", 1.0);
    Check(panel.rig.peak <= Limiter::kThreshold + 1e-4f,
          "every control at random: below the limiter",
          panel.rig.peak);
    Check(most <= Core::kNumVoices,
          "every control at random: no more grains than voices",
          double(most));

    // After all that, it still records and plays
    panel.c.knobs[Controls::S30] = 0.0f;
    panel.c.knobs[Controls::S31] = 0.5f;
    panel.c.knobs[Controls::S32] = 0.0f;
    panel.c.knobs[Controls::S33] = 0.5f;
    panel.c.knobs[Controls::S34] = 0.5f;
    panel.c.knobs[Controls::S35] = 1.0f;
    panel.c.knobs[Controls::S36] = 0.5f + 3.15f / 10.0f; // 1×
    panel.c.knobs[Controls::S37] = 0.75f;
    panel.c.switches             = {Controls::Centre, Controls::Down};
    panel.c.touched.fill(false);
    panel.c.pressure.fill(0.0f);
    panel.Run(0.5, Silence);
    panel.Record(2.0, Sine);
    const double kept = PanelRms(panel, 2.0, Silence);
    Check(kept > 0.05, "after it all: a new recording plays (rms)", kept);
}

// One grain of 1000 samples over a buffer of ones: its window, sample by
// sample
std::vector<float> Window(float shape)
{
    std::vector<float> ones(kRate, 1.0f);
    Buffer             buffer;
    buffer.Init(ones.data(), ones.size(), kRate);
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
    std::vector<float> window(1000, 0.0f), right(1000, 0.0f), sl(1000),
        sr(1000);
    Grain::Sums sums;
    grain.Render(1.0 / 1000.0,
                 window.data(),
                 right.data(),
                 sl.data(),
                 sr.data(),
                 window.size(),
                 sums);
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

// The output's RMS over seconds of silent input
double Rms(Rig& r, double seconds)
{
    float        l[kBlock] = {}, o1[kBlock], o2[kBlock];
    const float* in[2]     = {l, l};
    float*       out[2]    = {o1, o2};
    double       sum       = 0.0;
    const size_t blocks    = size_t(seconds * kRate / kBlock);
    for(size_t b = 0; b < blocks; b++)
    {
        r.core.Process(in, out, kBlock);
        for(size_t i = 0; i < kBlock; i++)
            sum += 0.5 * (double(o1[i]) * o1[i] + double(o2[i]) * o2[i]);
    }
    return std::sqrt(sum / double(blocks * kBlock));
}

// Mute: the grains silent within a few ms, the reverb ringing on
void Mute()
{
    Rig r(2.0);
    r.Fill(Sine);
    Core::Params p;
    p.density = 40.0f;
    r.core.SetParams(p);
    const double playing = Rms(r, 0.5);
    p.mute               = true;
    r.core.SetParams(p);
    Rms(r, 0.01);
    Check(Rms(r, 0.2) < 1e-4 && playing > 0.01,
          "P01 held, no reverb: silent",
          Rms(r, 0.2));
    p.mute = false;
    r.core.SetParams(p);
    Rms(r, 0.01);
    Check(Near(Rms(r, 0.5), playing, 0.5 * playing),
          "P01 released: the grains as before",
          Rms(r, 0.5));

    p.reverb = 0.8f;
    r.core.SetParams(p);
    Rms(r, 1.0);
    p.mute = true;
    r.core.SetParams(p);
    const double tail = Rms(r, 0.25);
    Rms(r, 2.75);
    const double later = Rms(r, 0.25);
    Check(tail > 0.01 && later < 0.5 * tail,
          "P01 held, reverb: the tail rings on, fading",
          later / tail);
}

// Grains started in the next seconds
size_t Starts(Rig& r, double seconds)
{
    size_t                               n = 0;
    std::array<double, Core::kNumVoices> last{};
    last.fill(2.0);
    r.Run(seconds, Silence, [&](size_t) {
        const auto& grains = r.core.Grains();
        for(size_t i = 0; i < grains.size(); i++)
        {
            const double now
                = grains[i].IsActive() ? grains[i].Progress() : 2.0;
            n += grains[i].IsActive() && now < last[i];
            last[i] = now;
        }
    });
    return n;
}

// End: the sand runs out and stops, turns the glass over, or starts again.
// Speed the other way or turned by hand, a glass run out runs back. A jump: a
// grain at once, whatever the density.
void Ends()
{
    Rig r(1.0);
    r.Fill(Sine);
    Core::Params p;
    p.speed   = 4.0f;
    p.density = 50.0f;
    p.end     = End::Once;
    r.core.SetParams(p);
    r.core.Jump(0.0f);
    r.Run(0.5, Silence);
    const size_t after = Starts(r, 0.5);
    Check(r.core.SandLeft() == 0.0f && after == 0,
          "once: run out, no more grains (starts)",
          double(after));
    p.speed = -4.0f;
    r.core.SetParams(p);
    r.Run(0.1, Silence);
    Check(r.core.PlayheadPhase() < 0.7 && r.core.SandLeft() > 0.0f,
          "once, run out, speed reversed: runs back",
          r.core.PlayheadPhase());
    p.speed = 4.0f;
    r.core.SetParams(p);
    r.Run(0.5, Silence);
    r.core.Turn();
    r.Run(0.1, Silence);
    Check(r.core.PlayheadPhase() < 0.7 && r.core.Heading() == -1,
          "once, run out, turned by hand: runs back",
          r.core.PlayheadPhase());
    r.core.Turn();

    p.speed = 4.0f;
    p.end   = End::Turn;
    r.core.SetParams(p);
    r.core.Jump(0.0f);
    r.Run(0.2, Silence);
    const double before = r.core.PlayheadPhase();
    r.Run(0.125, Silence);
    Check(before > 0.5 && r.core.PlayheadPhase() < before,
          "turn: at the end, it turns over and runs back",
          r.core.PlayheadPhase());
    r.Run(0.2, Silence);
    const double back = r.core.PlayheadPhase();
    r.Run(0.05, Silence);
    Check(r.core.PlayheadPhase() > back, "turn: and over again", back);

    p.end     = End::Once;
    p.density = 0.0f;
    r.core.SetParams(p);
    r.Run(1.0, Silence);
    r.core.Jump(0.5f);
    const size_t dropped = Starts(r, 0.2);
    Check(dropped == 1,
          "a jump at no density, run out: one grain",
          double(dropped));

    // A jump: the starts counted from it, at the point jumped to
    p.speed    = 0.0f;
    p.freeze   = true;
    p.end      = End::Loop;
    p.density  = 10.0f;
    p.grain_ms = 10.0f;
    r.core.SetParams(p);
    r.Run(0.33, Silence);
    r.core.Jump(0.25f);
    const size_t first = Starts(r, 0.05);
    const size_t gap   = Starts(r, 0.04);
    const size_t next  = Starts(r, 0.02);
    Check(first == 1 && gap == 0 && next == 1
              && Near(r.core.PlayheadPhase(), 0.25, 1e-3),
          "a jump: a grain at once, the next an interval later",
          r.core.PlayheadPhase());
}

// Turned over by hand, the playhead runs the other way from where it is, and
// the grains read backwards
void Turned()
{
    Rig r(1.0);
    r.Fill(Sine);
    Core::Params p;
    p.speed   = 1.0f;
    p.density = 50.0f;
    r.core.SetParams(p);
    r.core.Jump(0.5f);
    r.Run(0.1, Silence);
    const double before = r.core.PlayheadPhase();
    r.core.Turn();
    r.Run(0.1, Silence);
    bool backwards = true;
    for(const auto& g : r.core.Grains())
        backwards = backwards && (!g.IsActive() || g.Settings().rate < 0.0f);
    Check(r.core.PlayheadPhase() < before && backwards,
          "turned: runs back from where it is, grains reversed",
          r.core.PlayheadPhase());
    r.core.Turn();
    r.Run(0.1, Silence);
    Check(r.core.Heading() == 1 && r.core.PlayheadPhase() > before - 0.1,
          "turned again: upright",
          r.core.PlayheadPhase());
}

// The selection round the end of the recording, and its length drawn per pass
void Selection()
{
    Rig r(1.0);
    r.Fill(Sine);
    Core::Params p;
    p.start    = 0.75f;
    p.length   = 0.5f;
    p.density  = 80.0f;
    p.grain_ms = 10.0f;
    r.core.SetParams(p);
    double tail = 0.0, head = 0.0, outside = 0.0;
    r.Run(2.0, Silence, [&](size_t) {
        for(const auto& g : r.core.Grains())
        {
            if(!g.IsActive() || g.Progress() > 0.05)
                continue;
            double x = g.Settings().position / double(r.core.SoundSamples());
            x -= std::floor(x);
            (x >= 0.75 ? tail : x < 0.25 ? head : outside) += 1.0;
        }
    });
    Check(tail > 0.0 && head > 0.0 && outside == 0.0,
          "position 0.75, length 0.5: the last quarter, then the first",
          outside);

    // A new length keeps the playhead where it is in the recording; past the
    // new end, it starts a new pass
    p.start  = 0.0f;
    p.length = 1.0f;
    p.speed  = 1.0f;
    r.core.Jump(0.0f);
    r.core.SetParams(p);
    r.Run(0.3 * r.core.SoundSamples() / kRate, Silence);
    p.speed = 0.0f;
    r.core.SetParams(p);
    r.Run(0.01, Silence);
    const double was = r.core.PlayheadPhase();
    p.length         = 0.5f;
    r.core.SetParams(p);
    r.Run(0.01, Silence);
    Check(Near(r.core.PlayheadPhase(), 2.0 * was, 1e-6),
          "length halved: the playhead stays in the recording (phase)",
          r.core.PlayheadPhase());
    p.length = 0.2f;
    r.core.SetParams(p);
    r.Run(0.01, Silence);
    const double wrapped = 5.0 * was - std::floor(5.0 * was);
    Check(Near(r.core.PlayheadPhase(), wrapped, 1e-3),
          "length short of the playhead, loop: round again (phase)",
          r.core.PlayheadPhase());

    // Frozen: the point holds. S30 moves it with the selection, a new length
    // keeps it in the recording.
    p.length = 1.0f;
    p.speed  = 1.0f;
    r.core.SetParams(p);
    r.Run(0.3 * r.core.SoundSamples() / kRate, Silence);
    p.freeze = true;
    r.core.SetParams(p);
    r.Run(0.01, Silence);
    const double point = r.core.PlayheadPhase();
    r.Run(0.2, Silence);
    Check(r.core.PlayheadPhase() == point, "frozen: the point holds", point);
    p.start = 0.25f;
    r.core.SetParams(p);
    r.Run(0.01, Silence);
    Check(r.core.PlayheadPhase() == point,
          "frozen, S30 moved: the point moves with the selection (phase)",
          r.core.PlayheadPhase());
    p.length = 0.5f;
    r.core.SetParams(p);
    r.Run(0.01, Silence);
    Check(Near(r.core.PlayheadPhase(), 2.0 * point, 1e-6),
          "frozen, length halved: the point stays in the recording (phase)",
          r.core.PlayheadPhase());
}

// A new recording pours in over the one before: each new grain reads it with
// odds rising to 1 over kPourSeconds
void Pour()
{
    Rig r(2.0);
    r.Fill(Sine);
    Core::Params p;
    p.density  = 100.0f;
    p.grain_ms = 20.0f;
    r.core.SetParams(p);
    r.Run(0.5, Sine);
    r.Record(1.0, [](double t) {
        return 0.5f * float(std::sin(2.0 * M_PI * 440.0 * t));
    });
    const Buffer* now = &r.core.Sound();
    // New grains on the recording before, in each half second
    std::array<double, 6> before{}, all{};
    size_t                block = 0;
    r.Run(3.0, Silence, [&](size_t) {
        const size_t part = std::min(
            size_t(double(block++) * kBlock / kRate / 0.5), before.size() - 1);
        for(const auto& g : r.core.Grains())
        {
            if(!g.IsActive() || g.Progress() > 0.05)
                continue;
            all[part] += 1.0;
            if(g.Source() != now)
                before[part] += 1.0;
        }
    });
    Check(before[0] / all[0] > 0.6,
          "pour: at first mostly the recording before (share)",
          before[0] / all[0]);
    Check(before[2] / all[2] < before[0] / all[0] && before[2] > 0.0,
          "pour: then less of it (share)",
          before[2] / all[2]);
    Check(before[4] == 0.0 && before[5] == 0.0 && all[5] > 0.0,
          "pour: after 2 s, all the new one",
          before[4]);
}

// Frozen at the start: each grain moved by the amount, evenly
void Randomness()
{
    // Furthest position read, of the selection, and the mean
    const auto spread = [](float amount, double* mean) {
        Rig r(2.0);
        r.Fill(Sine);
        Core::Params p;
        p.freeze   = true;
        p.density  = 40.0f;
        p.grain_ms = 20.0f;
        p.rand_pos = amount;
        r.core.SetParams(p);
        double sum = 0.0, furthest = 0.0, n = 0.0;
        r.Run(2.0, Silence, [&](size_t) {
            for(const auto& g : r.core.Grains())
            {
                if(!g.IsActive() || g.Progress() > 0.1)
                    continue;
                const double x
                    = g.Settings().position / double(r.core.SoundSamples());
                sum += x;
                furthest = std::max(furthest, x);
                n++;
            }
        });
        *mean = sum / n;
        return furthest;
    };
    double mean = 0.0;
    double far  = spread(1.0f, &mean);
    Check(Near(mean, 0.5, 0.1) && far > 0.9,
          "position random up: even over the selection (mean)",
          mean);
    // Within 0.15 either way, round the selection
    const auto nearest = [](float amount) {
        Rig r(2.0);
        r.Fill(Sine);
        Core::Params p;
        p.freeze   = true;
        p.start    = 0.5f;
        p.length   = 0.5f;
        p.density  = 40.0f;
        p.grain_ms = 20.0f;
        p.rand_pos = amount;
        r.core.SetParams(p);
        double furthest = 0.0, before = 0.0;
        r.Run(2.0, Silence, [&](size_t) {
            for(const auto& g : r.core.Grains())
            {
                if(!g.IsActive() || g.Progress() > 0.1)
                    continue;
                double x = g.Settings().position / double(r.core.SoundSamples())
                           - 0.5;
                x              = 2.0 * x;
                const double d = std::min(x, 1.0 - x);
                furthest       = std::max(furthest, d);
                before += x > 0.5;
            }
        });
        return before > 0.0 ? furthest : -1.0;
    };
    const double near = nearest(0.3f);
    Check(near > 0.1 && near <= 0.15 + 0.01,
          "position random 0.3: within 0.15 either way, round the selection",
          near);
    far = spread(0.0f, &mean);
    Check(far < 0.01, "position random off: all at the playhead", far);
}

// Intervals between starts, of the mean: the shortest, the longest and their
// spread
void Starts()
{
    const auto intervals
        = [](bool random, double* shortest, double* longest, double* mean) {
              Rig r(2.0);
              r.Fill(Sine);
              Core::Params p;
              p.density       = 20.0f;
              p.grain_ms      = 10.0f;
              p.random_starts = random;
              r.core.SetParams(p);
              // A voice newly active, or further back than it was: a start
              std::vector<double>                  starts;
              std::array<double, Core::kNumVoices> last{};
              last.fill(2.0);
              r.Run(20.0, Silence, [&](size_t b) {
                  const auto& grains = r.core.Grains();
                  for(size_t i = 0; i < grains.size(); i++)
                  {
                      const double now
                          = grains[i].IsActive() ? grains[i].Progress() : 0.0;
                      if(grains[i].IsActive() && now < last[i])
                          starts.push_back(double(b) * kBlock / kRate);
                      last[i] = grains[i].IsActive() ? now : 2.0;
                  }
              });
              *shortest = 1e9;
              *longest  = 0.0;
              // From the second start: the first follows SetParams
              for(size_t i = 2; i < starts.size(); i++)
              {
                  const double d = (starts[i] - starts[i - 1]) * 20.0;
                  *shortest      = std::min(*shortest, d);
                  *longest       = std::max(*longest, d);
              }
              *mean = (starts.back() - starts[1]) * 20.0
                      / double(starts.size() - 2);
          };
    double shortest, longest, mean;
    intervals(false, &shortest, &longest, &mean);
    Check(shortest > 0.95 && longest < 1.05,
          "regular starts: periodic (shortest interval, of the mean)",
          shortest);
    intervals(true, &shortest, &longest, &mean);
    Check(Near(mean, 1.0, 0.1) && shortest < 0.3 && longest > 2.05,
          "random starts: the same mean, a long tail past 2 (longest)",
          longest);
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
    p.density       = Core::kMaxDensity;
    p.grain_ms      = 4000.0f;
    p.random_starts = true;
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
    Check(most == Core::kNumVoices && r.core.Dropped() > 0,
          "the most density, long grains: all voices, the rest dropped (count)",
          double(r.core.Dropped()));
    Check(sum / (3.0 * kRate / kBlock) > 10.0,
          "the most density, long grains: whole windows (mean of the windows)",
          sum / (3.0 * kRate / kBlock));
    bool whole = true;
    for(const auto& grain : r.core.Grains())
        whole = whole && grain.Settings().size == 1.0f;
    Check(whole, "the most density, long grains: sizes kept", 1.0);
    Check(r.finite, "the most density, long grains: output finite", 1.0);
}

// 16 grains at once, regular or random: as many sounding, on average
void Cloud()
{
    for(bool random : {false, true})
    {
        Rig r(2.0);
        r.Fill(Sine);
        Core::Params p;
        p.grain_ms      = 100.0f;
        p.density       = 16.0f * 1000.0f / p.grain_ms;
        p.random_starts = random;
        r.core.SetParams(p);
        r.Run(0.5, Silence);
        double sum = 0.0, n = 0.0;
        r.Run(4.0, Silence, [&](size_t) {
            for(const auto& grain : r.core.Grains())
                sum += grain.IsActive();
            n++;
        });
        Check(Near(sum / n, 16.0, 1.5) && r.finite,
              random ? "16 at once, random: 16 sounding (mean)"
                     : "16 at once, regular: 16 sounding (mean)",
              sum / n);
    }
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
    c.knobs[Controls::S35] = 1.0f;
    c.knobs[Controls::S36] = 0.5f + 3.15f / 10.0f; // on 1×
    c.switches             = {Controls::Centre, Controls::Down};
    ui.Process(c, r.core);
    const Core::Params& p = r.core.CurrentParams();
    Check(Near(p.speed, 1.0, 1e-6) && !p.freeze && p.end == End::Loop,
          "S36 on its third stop up, S07 down: speed 1 forward, loop",
          p.speed);
    Check(Near(p.pitch, 1.0, 1e-6) && p.window_shape == 0.5f && p.mix == 1.0f,
          "S33, S34 centre: unison, Hann, all wet",
          p.pitch);
    Check(p.start == 0.0f && p.length == 1.0f,
          "S30 bottom, S35 top: the whole recording",
          p.length);

    // S37, Density: none at the centre, regular below, random above, 1/16 to
    // 16 grains at once at the size
    const auto at = [&](float x) {
        c.knobs[Controls::S37] = x;
        ui.Process(c, r.core);
        return p;
    };
    const Core::Params closed  = at(0.5f);
    const Core::Params rare    = at(0.5f + CoreUI::kDensityDetent + 1e-4f);
    const Core::Params regular = at(0.0f);
    const Core::Params cloud   = at(1.0f);
    const auto         overlap = [](const Core::Params& q) {
        return q.density * q.grain_ms / 1000.0f;
    };
    Check(closed.density == 0.0f, "S37 centre: none", closed.density);
    Check(Near(overlap(rare), 1.0 / 16.0, 1e-3) && rare.random_starts,
          "S37 just above the centre: random, 1/16 grain at once",
          overlap(rare));
    Check(Near(overlap(regular), 16.0, 1e-2) && !regular.random_starts,
          "S37 bottom: regular, 16 grains at once",
          overlap(regular));
    Check(Near(overlap(cloud), 16.0, 1e-2) && cloud.random_starts,
          "S37 top: random, 16 grains at once",
          overlap(cloud));
    Check(rare.reverb == cloud.reverb && rare.grain_ms == cloud.grain_ms
              && cloud.spread == 0.0f,
          "S37: the reverb, size and spray their own",
          rare.reverb);
    c.knobs[Controls::S31] = 0.1f;
    ui.Process(c, r.core);
    Check(Near(overlap(p), 16.0, 1e-2) && p.grain_ms < cloud.grain_ms,
          "S37 top, smaller grains: more per second, as many at once",
          p.density);
    c.knobs[Controls::S31] = 0.5f;
    ui.Process(c, r.core);

    // S31, Size: 5 ms to 2 s, 100 ms at the centre, at most half the recording
    Check(Near(p.grain_ms, 100.0, 0.5), "S31 centre: 100 ms", p.grain_ms);
    c.knobs[Controls::S31] = 0.0f;
    ui.Process(c, r.core);
    const float small      = p.grain_ms;
    c.knobs[Controls::S31] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(small, CoreUI::kMinGrainMs, 0.01)
              && Near(p.grain_ms, 500.0 * r.core.SoundSamples() / kRate, 0.5),
          "S31: 5 ms up to half the recording (ms)",
          p.grain_ms);
    c.knobs[Controls::S31] = 0.5f;

    // S32, Spray: plain to scattered, evenly, the starts Density's
    c.knobs[Controls::S37] = 0.75f;
    ui.Process(c, r.core);
    Check(p.rand_pos == 0.0f && p.rand_size == 0.0f && p.rand_pitch == 0.0f
              && p.rand_gain == 0.0f && p.spread == 0.0f,
          "S32 bottom: plain",
          p.spread);
    c.knobs[Controls::S32] = 0.5f;
    ui.Process(c, r.core);
    Check(p.rand_pos > 0.0f && p.rand_pitch > 0.0f && p.random_starts,
          "S32 half: places and a little pitch, the starts as set",
          p.rand_pos);
    c.knobs[Controls::S32] = 1.0f;
    ui.Process(c, r.core);
    Check(p.rand_pos == 1.0f && p.rand_pitch == 1.0f && p.spread == 1.0f
              && p.rand_gain == 1.0f && p.rand_send == 1.0f,
          "S32 top: the whole selection, an octave either way, pan widest",
          p.rand_pitch);
    c.knobs[Controls::S32] = 0.0f;

    // S33, Pitch: stops at the fifth and two octaves
    c.knobs[Controls::S33] = 0.5f + 4.15f / 12.0f; // on the fifth
    ui.Process(c, r.core);
    Check(Near(p.pitch, std::exp2(7.0 / 12.0), 1e-3),
          "S33: a stop at a fifth",
          p.pitch);
    c.knobs[Controls::S33] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(p.pitch, 0.25, 1e-3), "S33 bottom: two octaves down", p.pitch);
    c.knobs[Controls::S33] = 0.5f;

    // S34, Shape: the window, reverse percussive to percussive
    c.knobs[Controls::S34] = 0.0f;
    ui.Process(c, r.core);
    Check(p.window_shape == 0.0f, "S34 down: reverse percussive", 0.0);
    c.knobs[Controls::S34] = 1.0f;
    ui.Process(c, r.core);
    Check(p.window_shape == 1.0f, "S34 up: percussive", p.window_shape);
    c.knobs[Controls::S34] = 0.5f;

    // S36, Speed: held at the centre, either way a quarter, a half, 1, 2, 4
    const auto speed = [&](float stop) {
        c.knobs[Controls::S36] = 0.5f + (stop + 0.15f) / 10.0f;
        ui.Process(c, r.core);
        return p.speed;
    };
    Check(Near(speed(1.0f), 0.25, 1e-6) && Near(speed(2.0f), 0.5, 1e-6),
          "S36 first and second stops: a quarter and half speed",
          p.speed);
    c.knobs[Controls::S36] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.speed, 4.0, 1e-6), "S36 top: 4 times", p.speed);
    c.knobs[Controls::S36] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(p.speed, -4.0, 1e-6), "S36 bottom: 4 times reverse", p.speed);
    c.knobs[Controls::S36] = 0.5f;
    ui.Process(c, r.core);
    Check(p.freeze && p.speed == 0.0f, "S36 centre: held", p.speed);
    c.knobs[Controls::S36] = 0.5f - 3.15f / 10.0f;
    ui.Process(c, r.core);
    Check(Near(p.speed, -1.0, 1e-6) && !p.freeze,
          "S36 third stop down: 1 reverse",
          p.speed);
    c.knobs[Controls::S36] = 0.5f + 3.15f / 10.0f;

    // S07, End
    c.switches[Controls::S07] = Controls::Up;
    ui.Process(c, r.core);
    Check(p.end == End::Once, "S07 up: once", 0);
    c.switches[Controls::S07] = Controls::Centre;
    ui.Process(c, r.core);
    Check(p.end == End::Turn, "S07 centre: turn", 1);
    c.switches[Controls::S07] = Controls::Down;
    ui.Process(c, r.core);

    // S09, Reverb: off, short, long
    c.switches[Controls::S09] = Controls::Down;
    ui.Process(c, r.core);
    const float off           = p.reverb;
    c.switches[Controls::S09] = Controls::Up;
    ui.Process(c, r.core);
    Check(off == 0.0f && p.reverb == CoreUI::kReverbLong,
          "S09: down no reverb, up long",
          p.reverb);
    c.switches[Controls::S09] = Controls::Centre;
    ui.Process(c, r.core);
    Check(p.reverb == CoreUI::kReverbShort, "S09 centre: short", p.reverb);

    // The LED glows with the grains, dimmer as the sand runs out
    auto press = [&](size_t pad, bool on) {
        c.touched[pad] = on;
        ui.Process(c, r.core);
    };
    c.knobs[Controls::S37] = 1.0f;
    press(CoreUI::kPlacePads[0], true);
    press(CoreUI::kPlacePads[0], false);
    r.Run(0.1, Silence);
    const float full = ui.Led(100000, r.core);
    r.Run(0.8, Silence);
    const float less = ui.Led(100000, r.core);
    Check(full > 0.0f && less < full, "LED: dims as the sand runs out", less);

    // P01 held: mute. P00 held: Spray at its top. P02 held: the playhead stops.
    press(CoreUI::kMutePad, true);
    Check(p.mute, "P01 held: mute", 1);
    press(CoreUI::kMutePad, false);
    Check(!p.mute, "P01 released: sound", 0);
    press(CoreUI::kShakePad, true);
    Check(p.rand_pos == 1.0f && p.spread == 1.0f, "P00 held: shaken", 1);
    press(CoreUI::kShakePad, false);
    Check(p.rand_pos == 0.0f, "P00 released: Spray as set", 0);
    press(CoreUI::kHoldPad, true);
    r.Run(0.01, Silence);
    const double point = r.core.PlayheadPhase();
    r.Run(0.2, Silence);
    Check(p.freeze && r.core.PlayheadPhase() == point,
          "P02 held: the playhead stops",
          r.core.PlayheadPhase());
    press(CoreUI::kHoldPad, false);
    r.Run(0.2, Silence);
    Check(!p.freeze && r.core.PlayheadPhase() > point,
          "P02 released: runs on from there",
          r.core.PlayheadPhase());

    // P03-P09, the places: the selection in sevenths, left to right. A touch:
    // there with a grain at once. Held: stays there. Released: runs on.
    c.knobs[Controls::S37] = 0.5f;
    ui.Process(c, r.core);
    r.Run(0.5, Silence);
    press(CoreUI::kPlacePads[3], true);
    const size_t one = Starts(r, 0.1);
    Check(one == 1 && Near(r.core.PlayheadPhase(), 3.0 / 7.0, 1e-3),
          "place 4 at no density: one grain, at the fourth seventh",
          r.core.PlayheadPhase());
    press(CoreUI::kPlacePads[3], false);
    const size_t none = Starts(r, 0.1);
    Check(none == 0, "place 4 released, no density: no more grains", none);
    c.knobs[Controls::S37] = 0.75f;
    press(CoreUI::kPlacePads[6], true);
    r.Run(0.3, Silence);
    Check(Near(r.core.PlayheadPhase(), 6.0 / 7.0, 1e-3),
          "place 7 held: the playhead stays at the last seventh",
          r.core.PlayheadPhase());
    press(CoreUI::kPlacePads[6], false);
    r.Run(0.1, Silence);
    Check(r.core.PlayheadPhase() > 6.0 / 7.0,
          "place 7 released: runs on from there",
          r.core.PlayheadPhase());

    // P11: the glass turned over by hand, once a touch
    press(CoreUI::kTurnPad, true);
    r.Run(0.1, Silence);
    const double turned = r.core.PlayheadPhase();
    r.Run(0.1, Silence);
    Check(r.core.Heading() == -1 && r.core.PlayheadPhase() < turned,
          "P11: turned over, the sand runs back",
          r.core.PlayheadPhase());
    press(CoreUI::kTurnPad, false);
    press(CoreUI::kTurnPad, true);
    press(CoreUI::kTurnPad, false);
    r.Run(0.01, Silence);
    Check(r.core.Heading() == 1, "P11 again: upright", 1);

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
    Record();
    Fuzz();
    Level();
    Windows();
    Mute();
    Ends();
    Turned();
    Selection();
    Pour();
    Randomness();
    Starts();
    Pan();
    Voices();
    Cloud();
    Mapping();
    std::printf("%s\n",
                failures ? "Some checks failed." : "All checks passed.");
    return failures ? 1 : 0;
}
