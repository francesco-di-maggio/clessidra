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
        c.knobs[Controls::S37] = 0.75f; // a stream, 28 grains per second
        c.switches             = {Controls::Up, Controls::Down}; // loop
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

// P11: tap keeps the last seconds, hold the touch, both into the buffer not
// playing
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
            panel.c.switches[Controls::S07]
                = Controls::Position(int(next() * 3.0f) % 3);
        if(next() < 0.1f)
            panel.c.switches[Controls::S09]
                = Controls::Position(int(next() * 3.0f) % 3);
        for(size_t pad : {CoreUI::kMutePad,
                          CoreUI::kSeedPad,
                          CoreUI::kRestartPad,
                          CoreUI::kFlipPad})
        {
            panel.c.touched[pad]  = next() < 0.1f;
            panel.c.pressure[pad] = next();
        }
        const size_t pad      = CoreUI::kNotePads[size_t(next() * 7.0f) % 7];
        panel.c.touched[pad]  = next() < 0.3f;
        panel.c.pressure[pad] = next();
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
    Check(most < Core::kNumVoices,
          "every control at random: a voice stays free",
          double(most));

    // After all that, it still records and plays
    panel.c.knobs[Controls::S30] = 0.0f;
    panel.c.knobs[Controls::S31] = 0.5f;
    panel.c.knobs[Controls::S32] = 0.0f;
    panel.c.knobs[Controls::S33] = 0.5f;
    panel.c.knobs[Controls::S34] = 0.5f;
    panel.c.knobs[Controls::S35] = 1.0f;
    panel.c.knobs[Controls::S36] = 0.5f + 1.15f / 6.0f; // 1×
    panel.c.knobs[Controls::S37] = 0.75f;
    panel.c.switches             = {Controls::Up, Controls::Down};
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

// The filter's gain at hz, dB, on a sine after a second to settle
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

void Filter()
{
    Check(Near(Gain(4000, 200, 1.5f, true), -52.0, 1.0),
          "filter: lowpass 200 Hz at 4 kHz (dB)",
          Gain(4000, 200, 1.5f, true));
    Check(Near(Gain(200, 2000, 1.5f, false), -40.0, 1.0),
          "filter: highpass 2 kHz at 200 Hz (dB)",
          Gain(200, 2000, 1.5f, false));
    Check(std::fabs(Gain(10000, 20000, 0.707f, true)) < 0.05,
          "filter: off (lowpass 20 kHz) at 10 kHz (dB)",
          Gain(10000, 20000, 0.707f, true));
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

size_t Starts(Rig& r, double seconds);

// Notes: while held, grains at the playhead at their pitch, none once the
// playhead has run out
void Notes()
{
    Rig r(1.0);
    r.Fill(Sine);
    Core::Params p;
    p.density  = 0.0f;
    p.pitch    = 0.5f;
    p.notes[2] = {50.0f, 1.5f, true};
    r.core.SetParams(p);
    r.Run(0.1, Silence);
    bool   at_pitch = true;
    size_t sounding = 0;
    for(const auto& g : r.core.Grains())
    {
        if(!g.IsActive())
            continue;
        sounding++;
        at_pitch = at_pitch && Near(g.Settings().rate, 0.75, 1e-6);
    }
    Check(sounding > 0 && at_pitch && r.core.NoteOn(2),
          "note held: grains at Pitch times its pitch",
          double(sounding));
    p.notes[2].held = false;
    r.core.SetParams(p);
    r.Run(0.3, Silence); // the last grains end
    const size_t after = Starts(r, 0.2);
    Check(after == 0 && !r.core.NoteOn(2),
          "note released: no more grains",
          double(after));

    // Every note held, all at the most: together no more than the most
    p.density = Core::kMaxDensity;
    for(auto& note : p.notes)
        note = {Core::kMaxDensity, 1.0f, true};
    r.core.SetParams(p);
    const double per_second = double(Starts(r, 1.0));
    Check(Near(per_second, Core::kMaxDensity, 0.15 * Core::kMaxDensity),
          "all notes and the playhead at the most: they share it (per s)",
          per_second);
    for(auto& note : p.notes)
        note.held = false;
    p.density = 0.0f;

    p.end           = End::Once;
    p.speed         = 4.0f;
    p.notes[2].held = true;
    r.core.SetParams(p);
    r.core.Restart();
    r.Run(0.5, Silence);
    const size_t ended = Starts(r, 0.2);
    Check(ended == 0, "note held, the playhead run out: silent", double(ended));
    p.notes[2].held = false;
    p.end           = End::Loop;
    p.speed         = 1.0f;
    p.pitch         = 1.0f;

    // The playhead goes round the selection, and starts it again on Restart
    p.freeze  = false;
    p.speed   = 4.0f;
    p.density = 50.0f;
    r.core.SetParams(p);
    double least = 1.0, most = 0.0;
    r.Run(1.0, Silence, [&](size_t) {
        least = std::min(least, r.core.PlayheadPhase());
        most  = std::max(most, r.core.PlayheadPhase());
    });
    Check(least >= 0.0 && most < 1.0 && most > 0.9,
          "the playhead loops the selection",
          most);
    r.core.Restart();
    r.Run(0.001, Silence);
    Check(r.core.PlayheadPhase() < 0.01,
          "Restart: from the start",
          r.core.PlayheadPhase());
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
// Flicked the other way, a glass run out runs back. A drop: one grain.
void Ends()
{
    Rig r(1.0);
    r.Fill(Sine);
    Core::Params p;
    p.speed   = 4.0f;
    p.density = 50.0f;
    p.end     = End::Once;
    r.core.SetParams(p);
    r.core.Restart();
    r.Run(0.5, Silence);
    const size_t after = Starts(r, 0.5);
    Check(r.core.SandLeft() == 0.0f && after == 0,
          "once: run out, no more grains (starts)",
          double(after));
    p.speed = -4.0f;
    r.core.SetParams(p);
    r.Run(0.1, Silence);
    Check(r.core.PlayheadPhase() < 0.7 && r.core.SandLeft() > 0.0f,
          "once, run out, turned by hand: runs back",
          r.core.PlayheadPhase());

    p.end     = End::Once;
    p.density = 0.0f;
    r.core.SetParams(p);
    r.Run(1.0, Silence);
    r.core.Seed();
    const size_t dropped = Starts(r, 0.2);
    Check(
        dropped == 1, "a drop: one grain, run out and closed", double(dropped));
}

// RMS of samples [from, to) of b
double Rms(const Buffer& b, size_t from, size_t to)
{
    double sum = 0.0;
    for(size_t i = from; i < to; i++)
        sum += double(b.Samples()[i]) * b.Samples()[i];
    return std::sqrt(sum / double(std::max<size_t>(to - from, 1)));
}

// Flip: the grains fall into the bottom bulb; turned over, it plays, the last
// grain first, at the level they fell at
void Flips()
{
    Rig r(1.0);
    // Rising: what falls rises too, turned over it falls
    r.Fill([](double t) { return float(std::fmod(t, 1.0)) * Sine(t); });
    const double rising = Rms(r.core.Sound(), 0, r.core.SoundSamples());
    Core::Params p;
    p.density  = 100.0f;
    p.grain_ms = 20.0f;
    p.end      = End::Flip;
    r.core.SetParams(p);
    r.core.Restart();
    r.Run(0.5, Silence);
    Check(Near(r.core.Fallen(), 0.5, 0.02),
          "flip: half the sand fallen in half a second (s)",
          r.core.Fallen());
    r.Run(0.6, Silence);
    const Buffer& up = r.core.Sound();
    const size_t  n  = up.Length();
    Check(r.core.Flips() == 1 && Near(double(n) / kRate, 1.0, 0.05),
          "flip: run out, turned over, what fell plays (s)",
          double(n) / kRate);
    const double first = Rms(up, 0, n / 2), second = Rms(up, n / 2, n);
    Check(first > 1.5 * second,
          "flip: the last grain first, rising now falls (first / second)",
          first / second);
    const double level = Rms(up, 0, n) / rising;
    Check(level > 0.5 && level < 2.0,
          "flip: at the level it fell (over the recording)",
          level);

    // By hand, in Loop
    p.end = End::Loop;
    r.core.SetParams(p);
    r.Run(0.3, Silence);
    r.core.Flip();
    r.Run(0.01, Silence);
    Check(r.core.Flips() == 2 && r.core.PlayheadPhase() < 0.05,
          "flip by hand: turned over, from the start",
          double(r.core.Flips()));

    // Nothing fallen: it only starts again
    p.density = 0.0f;
    r.core.SetParams(p);
    r.Run(0.2, Silence); // the last grains end
    p.end = End::Flip;
    r.core.SetParams(p);
    r.core.Restart();
    r.Run(2.5, Silence);
    Check(r.core.Flips() == 2 && r.core.SoundSamples() > 0,
          "flip, nothing fallen: the sound stays",
          double(r.core.Flips()));

    // A new recording after flips: it still lands and plays
    p.density = 50.0f;
    r.core.SetParams(p);
    const uint32_t before = r.core.Recordings();
    r.Record(0.5, Sine);
    const double kept = Rms(r, 0.5);
    Check(r.core.Recordings() == before + 1 && kept > 0.01,
          "after flips: a new recording lands and plays (rms)",
          kept);
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
    r.core.Restart();
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

// As Beads, frozen at the start: up, each grain moved by the amount, evenly.
// Down, by a random part of three times it: mostly near, a long tail.
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
    far = spread(0.3f, &mean);
    Check(far <= 0.3 + 0.01, "position random 0.3: within 0.3", far);
    far = spread(-0.3f, &mean);
    Check(far > 0.6 && mean < 0.3,
          "position random -0.3: mostly near, a long tail (furthest)",
          far);
    far = spread(0.0f, &mean);
    Check(far < 0.01, "position random off: all at the playhead", far);
}

// Intervals between starts, of the mean: the shortest, the longest and their
// spread
void Starts()
{
    const auto intervals
        = [](float amount, double* shortest, double* longest, double* mean) {
              Rig r(2.0);
              r.Fill(Sine);
              Core::Params p;
              p.density      = 20.0f;
              p.grain_ms     = 10.0f;
              p.rand_density = amount;
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
    intervals(0.0f, &shortest, &longest, &mean);
    Check(shortest > 0.95 && longest < 1.05,
          "start randomness off: periodic (shortest interval, of the mean)",
          shortest);
    intervals(1.0f, &shortest, &longest, &mean);
    Check(Near(mean, 1.0, 0.1) && shortest < 0.2 && longest > 1.8
              && longest < 2.05,
          "start randomness up: an even spread, 0 to 2 (longest)",
          longest);
    intervals(-1.0f, &shortest, &longest, &mean);
    Check(Near(mean, 1.0, 0.1) && longest > 2.05,
          "start randomness down: a long tail past 2 (longest)",
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
    p.density      = Core::kMaxDensity;
    p.grain_ms     = 4000.0f;
    p.rand_density = -1.0f;
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
    c.knobs[Controls::S36] = 0.5f + 1.15f / 6.0f; // on 1×
    c.switches             = {Controls::Up, Controls::Down};
    ui.Process(c, r.core);
    const Core::Params& p = r.core.CurrentParams();
    Check(Near(p.speed, 1.0, 1e-6) && !p.freeze && p.end == End::Loop,
          "S36 on its first stop up, S07 down: speed 1 forward, loop",
          p.speed);
    Check(Near(p.pitch, 1.0, 1e-6) && p.window_shape == 0.5f && p.filter == 0.0f
              && p.mix == 1.0f,
          "S33, S34 centre: unison, Hann, no filter, all wet",
          p.pitch);
    Check(p.start == 0.0f && p.length == 1.0f,
          "S30 bottom, S35 top: the whole recording",
          p.length);

    // S37, Density: none, sparse, dense
    c.knobs[Controls::S37] = 0.0f;
    ui.Process(c, r.core);
    const Core::Params closed = p;
    c.knobs[Controls::S37]    = 0.1f;
    ui.Process(c, r.core);
    const Core::Params drops = p;
    c.knobs[Controls::S37]   = 1.0f;
    ui.Process(c, r.core);
    Check(closed.density == 0.0f && drops.density > 0.0f
              && drops.density < 1.0f,
          "S37 bottom: none, then single grains (per s)",
          drops.density);
    Check(Near(p.density, Core::kMaxDensity, 0.01),
          "S37 top: the most density",
          p.density);
    Check(
        drops.reverb > p.reverb && drops.grain_ms < p.grain_ms,
        "S37: sparse grains short in a big reverb, dense ones long and closer",
        drops.reverb);
    Check(Near(p.grain_ms, CoreUI::kDensityGrainMsHi, 0.5)
              && Near(closed.grain_ms, CoreUI::kDensityGrainMsLo, 0.01),
          "S31 centre: Density's size, 30 to 300 ms (ms)",
          p.grain_ms);
    Check(Near(p.spread, CoreUI::kCloudSpray, 1e-6) && p.rand_density < 0.0f
              && closed.spread == 0.0f,
          "S37 top: the cloud sprays by itself, S32 at 0",
          p.spread);

    // S31: two octaves either way of Density's
    c.knobs[Controls::S37] = 0.5f;
    ui.Process(c, r.core);
    const float flow_ms    = p.grain_ms;
    c.knobs[Controls::S31] = 0.0f;
    ui.Process(c, r.core);
    const float small      = p.grain_ms;
    c.knobs[Controls::S31] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(small, flow_ms / 4.0, 0.01)
              && Near(p.grain_ms, flow_ms * 4.0, 0.1),
          "S31: a quarter to four times Density's size (ms)",
          p.grain_ms);
    c.knobs[Controls::S37] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, 500.0 * r.core.SoundSamples() / kRate, 0.5),
          "S31, S37 top: at most half the recording (ms)",
          p.grain_ms);
    c.knobs[Controls::S31] = 0.5f;
    c.knobs[Controls::S37] = 0.5f;

    // S32, Spray: plain to disorder, pitch only over its top quarter
    ui.Process(c, r.core);
    Check(p.rand_density == 0.0f && p.rand_pos == 0.0f && p.rand_size == 0.0f
              && p.rand_shape == 0.0f && p.rand_pitch == 0.0f
              && p.spread == 0.0f,
          "S32 bottom: plain, periodic grains",
          p.spread);
    c.knobs[Controls::S32] = 0.5f;
    ui.Process(c, r.core);
    Check(p.rand_density == -1.0f && p.rand_pos < 0.0f && p.rand_pitch == 0.0f,
          "S32 half: random starts and places, no pitch yet",
          p.rand_pos);
    c.knobs[Controls::S32] = 1.0f;
    ui.Process(c, r.core);
    Check(p.rand_pos == -1.0f && p.rand_pitch < 0.0f && p.spread == 1.0f,
          "S32 top: the most spray, random pitch, pan at its widest",
          p.rand_pitch);
    c.knobs[Controls::S32] = 0.0f;

    // P11: a touch turns the glass over, once
    r.Run(0.5, Silence);
    const uint32_t flips        = r.core.Flips();
    c.touched[CoreUI::kFlipPad] = true;
    ui.Process(c, r.core);
    r.Run(0.01, Silence);
    ui.Process(c, r.core);
    r.Run(0.01, Silence);
    c.touched[CoreUI::kFlipPad] = false;
    ui.Process(c, r.core);
    r.Run(0.01, Silence);
    Check(r.core.Flips() == flips + 1,
          "P11: one flip a touch",
          double(r.core.Flips() - flips));

    // S33, Pitch: Beads' stops, the fifth and two octaves
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
    Check(p.window_shape == 1.0f && p.filter == 0.0f,
          "S34 up: percussive",
          p.window_shape);
    c.knobs[Controls::S34] = 0.5f;

    // S36, Speed: stopped at the centre, either way to 4 times. S07
    c.knobs[Controls::S36] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.speed, 4.0, 1e-6), "S36 top: 4 times", p.speed);
    c.knobs[Controls::S36] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(p.speed, -4.0, 1e-6), "S36 bottom: 4 times reverse", p.speed);
    c.knobs[Controls::S36] = 0.5f;
    ui.Process(c, r.core);
    Check(p.freeze && p.speed == 0.0f, "S36 centre: held", p.speed);
    c.knobs[Controls::S36] = 0.5f - 1.15f / 6.0f;
    ui.Process(c, r.core);
    Check(Near(p.speed, -1.0, 1e-6) && !p.freeze,
          "S36 first stop down: 1 reverse",
          p.speed);
    c.knobs[Controls::S36]    = 0.5f + 1.15f / 6.0f;
    c.switches[Controls::S07] = Controls::Up;
    ui.Process(c, r.core);
    Check(p.end == End::Once, "S07 up: once", 0);
    c.switches[Controls::S07] = Controls::Centre;
    ui.Process(c, r.core);
    Check(p.end == End::Flip, "S07 centre: flip", 1);
    c.switches[Controls::S07] = Controls::Down;
    ui.Process(c, r.core);

    // The LED glows with the grains, dimmer as the sand runs out
    auto press = [&](size_t pad, bool on) {
        c.touched[pad] = on;
        ui.Process(c, r.core);
    };
    c.knobs[Controls::S37] = 0.75f;
    ui.Process(c, r.core);
    press(CoreUI::kRestartPad, true);
    press(CoreUI::kRestartPad, false);
    r.Run(0.1, Silence);
    const float full = ui.Led(100000, r.core);
    r.Run(0.8, Silence);
    const float less = ui.Led(100000, r.core);
    Check(full > 0.0f && less < full, "LED: dims as the sand runs out", less);

    // A note pad: grains of its own at the playhead, in S09's scale above
    // Pitch. Pressure raises its density.
    const float  density = p.density;
    const double phase   = r.core.PlayheadPhase();
    const size_t key     = CoreUI::kNotePads[4];
    c.pressure[key]      = 0.3f;
    press(key, true);
    r.Run(0.001, Silence);
    Check(p.density == density && r.core.NoteOn(4)
              && Near(p.notes[4].density, density, 0.01),
          "note touched lightly: Density's, the playhead as it was",
          p.notes[4].density);
    Check(std::fabs(r.core.PlayheadPhase() - phase) < 0.01,
          "note touched: the playhead goes on",
          r.core.PlayheadPhase());
    Check(Near(p.notes[4].pitch, std::exp2(7.0 / 12.0), 1e-6),
          "S09 up, fifth pad: a fifth, major",
          p.notes[4].pitch);
    c.switches[Controls::S09] = Controls::Centre;
    ui.Process(c, r.core);
    Check(Near(p.notes[2].pitch, std::exp2(3.0 / 12.0), 1e-6),
          "S09 centre, third pad: a minor third",
          p.notes[2].pitch);
    c.switches[Controls::S09] = Controls::Down;
    ui.Process(c, r.core);
    Check(Near(p.notes[6].pitch, std::exp2(14.0 / 12.0), 1e-6),
          "S09 down, last pad: the ninth above the octave, pentatonic",
          p.notes[6].pitch);
    c.switches[Controls::S09] = Controls::Up;
    c.pressure[key]           = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.notes[4].density, Core::kMaxDensity, 0.01),
          "note pressed hard: the most density",
          p.notes[4].density);
    c.knobs[Controls::S37] = 0.0f;
    c.pressure[key]        = 0.3f;
    ui.Process(c, r.core);
    Check(p.notes[4].density > 1.0f,
          "note, Density at 0: at least half",
          p.notes[4].density);
    press(key, false);
    c.knobs[Controls::S37] = 0.75f;

    // P01 held: mute. P00: a seed. P02: from the start.
    press(CoreUI::kMutePad, true);
    Check(p.mute, "P01 held: mute", 1);
    press(CoreUI::kMutePad, false);
    Check(!p.mute, "P01 released: sound", 0);
    c.knobs[Controls::S37] = 0.0f;
    ui.Process(c, r.core);
    r.Run(0.5, Silence);
    press(CoreUI::kSeedPad, true);
    const size_t one = Starts(r, 0.1);
    press(CoreUI::kSeedPad, false);
    const size_t none = Starts(r, 0.1);
    Check(one == 1 && none == 0, "P00: one grain a touch", double(one));
    c.knobs[Controls::S37] = 0.75f;
    ui.Process(c, r.core);
    r.Run(0.3, Silence);
    press(CoreUI::kRestartPad, true);
    r.Run(0.001, Silence);
    Check(r.core.PlayheadPhase() < 0.01,
          "P02: the playhead from the start",
          r.core.PlayheadPhase());
    press(CoreUI::kRestartPad, false);

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
    Filter();
    Mute();
    Notes();
    Ends();
    Flips();
    Selection();
    Pour();
    Randomness();
    Starts();
    Pan();
    Voices();
    Mapping();
    std::printf("%s\n",
                failures ? "Some checks failed." : "All checks passed.");
    return failures ? 1 : 0;
}
