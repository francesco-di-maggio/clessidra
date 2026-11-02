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
        c.knobs[Controls::S37] = 1.0f;
        c.knobs[Controls::S35] = 1.0f;
        c.knobs[Controls::S36] = 0.5f;
        c.knobs[Controls::S34] = 0.755f; // periodic, 28 grains per second
        c.switches             = {Controls::Up, Controls::Centre};
    }

    // P11 held for seconds of the input, then until it lands
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

    // S37: dry passes the input as it is, wet only the grains
    auto  level = [](double) { return 0.25f; };
    Panel dry;
    dry.c.knobs[Controls::S37] = 0.0f;
    dry.Run(0.5, level);
    const double through = PanelRms(dry, 0.5, level);
    Check(Near(through, 0.25, 0.001),
          "S37 down: the input, dry, at its level (rms)",
          through);
    Panel wet;
    wet.Run(0.5, Sine);
    const double none = PanelRms(wet, 0.5, Sine);
    Check(none < 1e-6, "S37 up, nothing recorded: no dry signal (rms)", none);

    // A new recording: the LED flashes while held, the playhead restarts
    Panel led;
    led.c.touched[CoreUI::kRecordPad] = true;
    led.Run(0.5, Sine);
    const float on  = led.ui.Led(0, led.rig.core);
    const float off = led.ui.Led(CoreUI::kFlashMs, led.rig.core);
    Check(on == 1.0f && off == 0.0f, "recording: the LED flashes", off);

    // Recording again while the last one plays: no jump at the swap
    Panel again;
    again.c.knobs[Controls::S34]    = 1.0f;
    again.c.switches[Controls::S07] = Controls::Down;
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
        const size_t pad      = CoreUI::kSlicePads[size_t(next() * 7.0f) % 7];
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
    panel.c.knobs[Controls::S30]    = 0.0f;
    panel.c.knobs[Controls::S37]    = 1.0f;
    panel.c.knobs[Controls::S31]    = 0.5f;
    panel.c.knobs[Controls::S32]    = 0.5f;
    panel.c.knobs[Controls::S33]    = 0.5f;
    panel.c.knobs[Controls::S35]    = 1.0f;
    panel.c.knobs[Controls::S36]    = 0.5f;
    panel.c.knobs[Controls::S34]    = 0.755f;
    panel.c.switches[Controls::S07] = Controls::Down;
    panel.c.touched.fill(false);
    panel.c.pressure.fill(0.0f);
    panel.Run(0.5, Silence);
    panel.Record(2.0, Sine);
    const double kept = PanelRms(panel, 2.0, Silence);
    Check(kept > 0.05, "after it all: a new recording plays (rms)", kept);

    // A slice without width, as a torn parameter copy gave: nothing breaks
    Rig          zero(1.0);
    Core::Params none;
    zero.Fill(Sine);
    none.triggers[0] = {0.5f, 0.5f, Core::kMaxDensity, 1};
    zero.core.SetParams(none);
    zero.Run(0.5, Sine);
    Check(zero.finite && !zero.core.TriggerOn(0),
          "a slice without width: silent, output finite",
          1.0);
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

// Triggers: once to the end of their slice, while held, or round it while held
void Triggers()
{
    Rig r(1.0);
    r.Fill(Sine);
    Core::Params p;
    p.density = 0.0f;
    // Seconds a trigger sounds over slice 1/7 at speed 1, held for held seconds
    auto lasts = [&r, &p](Core::Trigger::Mode mode, double held, uint32_t id) {
        p.triggers[0] = {0.0f, 1.0f / 7.0f, 50.0f, id, mode, true};
        r.core.SetParams(p);
        double seconds = 0.0;
        while(seconds < 0.6)
        {
            if(seconds >= held && p.triggers[0].held)
            {
                p.triggers[0].held = false;
                r.core.SetParams(p);
            }
            r.Run(0.001, Silence);
            seconds += 0.001;
            if(!r.core.TriggerOn(0))
                break;
        }
        return seconds;
    };
    using Mode        = Core::Trigger::Mode;
    const double once = lasts(Mode::Once, 0.01, 1);
    Check(Near(once, 1.0 / 7.0, 0.005),
          "trigger once: through its slice, released early (s)",
          once);
    const double gate = lasts(Mode::Gate, 0.05, 2);
    Check(Near(gate, 0.05, 0.003), "trigger gate: while held (s)", gate);
    const double loop = lasts(Mode::Loop, 0.4, 3);
    Check(Near(loop, 0.4, 0.003),
          "trigger loop: round its slice while held, past its end (s)",
          loop);
    p.freeze          = true;
    const double cold = lasts(Mode::Once, 0.05, 4);
    Check(Near(cold, 0.05, 0.003),
          "trigger once, frozen: stops on release (s)",
          cold);

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

    // Length random: one length a pass, a new one at the next
    p.start       = 0.0f;
    p.speed       = 4.0f;
    p.rand_length = 1.0f;
    r.core.SetParams(p);
    double             last = r.core.PlayheadPhase();
    float              pass = r.core.PassLength();
    std::vector<float> lengths;
    bool               steady = true;
    // From the first new pass: the one before took the change of amount
    r.Run(4.0, Silence, [&](size_t) {
        const double now = r.core.PlayheadPhase();
        if(now < last)
            lengths.push_back(r.core.PassLength());
        else if(!lengths.empty() && r.core.PassLength() != pass)
            steady = false;
        pass = r.core.PassLength();
        last = now;
    });
    const auto [shortest, longest]
        = std::minmax_element(lengths.begin(), lengths.end());
    Check(steady && lengths.size() > 4 && *longest - *shortest > 0.2f,
          "length random: one length a pass, each pass its own (spread)",
          *longest - *shortest);
    p.rand_length = 0.0f;
    r.core.SetParams(p);
    r.Run(0.01, Silence);
    Check(r.core.PassLength() == 0.5f,
          "length random off: the length set",
          r.core.PassLength());
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
    c.knobs[Controls::S35] = 1.0f;
    c.switches             = {Controls::Up, Controls::Up};
    ui.Process(c, r.core);
    const Core::Params& p = r.core.CurrentParams();
    Check(Near(p.density,
               0.3 * std::pow(Core::kMaxDensity / 0.3, 0.48 / 0.98),
               0.01),
          "S34 centre: halfway, exponential",
          p.density);
    Check(Near(p.pitch, 1.0, 1e-6), "S33 centre: pitch (ratio)", p.pitch);
    Check(Near(p.speed, 1.0, 1e-6) && !p.freeze,
          "S36 centre, S09 up: speed 1 forward",
          p.speed);
    Check(p.rand_density == 0.0f && p.rand_pos == 0.0f && p.rand_size == 0.0f
              && p.rand_shape == 0.0f && p.rand_pitch == 0.0f
              && p.spread == 0.0f && p.window_shape == 0.5f,
          "power-up: no randomness, periodic starts, Hann",
          p.rand_density);
    Check(p.start == 0.0f && p.length == 1.0f,
          "S30 bottom, S35 top: the whole recording",
          p.length);
    c.knobs[Controls::S35] = 0.5f;
    c.knobs[Controls::S30] = 0.75f;
    ui.Process(c, r.core);
    Check(Near(p.start, 0.75, 1e-6) && Near(p.length, 0.5, 1e-6),
          "S35 half, S30 three quarters: from there, round the end",
          p.start);
    c.knobs[Controls::S30] = 0.0f;
    ui.Process(c, r.core);
    Check(p.start == 0.0f, "S30 bottom: from the start", p.start);
    c.knobs[Controls::S35] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(p.length, Core::kMinLength, 1e-6),
          "S35 bottom: the shortest selection",
          p.length);
    c.knobs[Controls::S30] = 0.0f;
    c.knobs[Controls::S35] = 1.0f;
    ui.Process(c, r.core);

    c.knobs[Controls::S34] = CoreUI::kDensityStop;
    ui.Process(c, r.core);
    Check(p.density == 0.0f, "S34 bottom: no grains", p.density);
    c.knobs[Controls::S34] = 0.03f;
    ui.Process(c, r.core);
    const Core::Params low = p;
    c.knobs[Controls::S34] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.density, Core::kMaxDensity, 0.01) && low.density > 0.0f
              && low.density < 1.0f && p.rand_density == 0.0f,
          "S34 up from the bottom: sparse to the most, periodic",
          p.density);
    Check(p.grain_ms == low.grain_ms && p.window_shape == low.window_shape
              && p.reverb == low.reverb && p.spread == 0.0f,
          "S34: density alone",
          p.grain_ms);
    c.knobs[Controls::S34] = 0.5f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, 200.0, 0.5), "S31 centre: 200 ms", p.grain_ms);
    c.knobs[Controls::S31] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, CoreUI::kMinGrainMs, 1e-3),
          "S31 bottom: 10 ms",
          p.grain_ms);
    c.knobs[Controls::S31] = 0.5f;
    ui.Process(c, r.core);

    // P00: the FX layer, each knob picked up where its value is
    Check(p.reverb == CoreUI::kReverbStart && p.filter == 0.0f,
          "FX at power-up: filter off, some reverb",
          p.reverb);
    c.touched[CoreUI::kFxPad] = true;
    c.knobs[Controls::S32]    = 0.9f; // away from the reverb's value
    ui.Process(c, r.core);
    Check(p.reverb == CoreUI::kReverbStart && p.window_shape == 0.5f,
          "P00 + S32 away from the reverb: nothing moves",
          p.reverb);
    c.knobs[Controls::S32] = 0.2f; // across it
    ui.Process(c, r.core);
    c.knobs[Controls::S32] = 0.6f;
    ui.Process(c, r.core);
    Check(Near(p.reverb, 0.6, 1e-6) && p.window_shape == 0.5f,
          "P00 + S32 picked up: reverb, shape stays",
          p.reverb);
    c.knobs[Controls::S31] = 0.0f; // from its value, at the centre
    ui.Process(c, r.core);
    Check(p.filter == -1.0f && Near(p.grain_ms, 200.0, 0.5),
          "P00 + S31 down: low-pass, size stays",
          p.filter);
    c.knobs[Controls::S37] = 0.9f;
    ui.Process(c, r.core);
    Check(Near(p.mix, 0.9, 1e-6), "P00 + S37: nothing on FX, dry/wet", p.mix);
    c.touched[CoreUI::kShiftPad] = true;
    c.knobs[Controls::S31]       = 0.5f;
    ui.Process(c, r.core);
    Check(p.filter == 0.0f && p.rand_size == 0.0f,
          "P00 and P11 held: FX",
          p.filter);
    c.touched[CoreUI::kFxPad] = false;
    c.knobs[Controls::S37]    = 0.5f;
    ui.Process(c, r.core);

    // P11: the shift layer, each knob's randomness
    c.knobs[Controls::S31] = 1.0f; // from its value, at the centre
    ui.Process(c, r.core);
    Check(p.rand_size == 1.0f && Near(p.grain_ms, 200.0, 0.5),
          "P11 + S31: size randomness, size stays",
          p.rand_size);
    c.knobs[Controls::S33] = 1.0f;
    ui.Process(c, r.core);
    Check(p.rand_pitch == 1.0f && p.pitch == 1.0f,
          "P11 + S33: pitch randomness, pitch stays",
          p.rand_pitch);
    const float shown = ui.Led(0, r.core);
    Check(shown == 1.0f, "P11 held: the LED shows the knob last turned", shown);
    c.touched[CoreUI::kShiftPad] = false;
    ui.Process(c, r.core);
    Check(p.pitch == 1.0f && Near(p.mix, 0.5, 1e-6),
          "P11 released, knobs away: their own values unchanged",
          p.pitch);
    c.knobs[Controls::S31] = 0.5f;
    c.knobs[Controls::S32] = 0.5f;
    c.knobs[Controls::S33] = 0.5f;
    ui.Process(c, r.core);

    c.knobs[Controls::S36] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.speed, 4.0, 1e-6), "S36 top: 4 times", p.speed);
    c.knobs[Controls::S36] = 0.0f;
    ui.Process(c, r.core);
    Check(p.freeze, "S36 bottom: stopped", p.speed);
    c.knobs[Controls::S36]    = 0.5f;
    c.switches[Controls::S09] = Controls::Down;
    ui.Process(c, r.core);
    Check(Near(p.speed, -1.0, 1e-6), "S09 down: reverse", p.speed);
    c.switches[Controls::S09] = Controls::Centre;
    ui.Process(c, r.core);
    Check(p.freeze, "S09 centre: freeze", p.speed);
    c.switches[Controls::S09] = Controls::Up;

    c.knobs[Controls::S33] = 0.5f + 3.0f / 14.0f; // on the fifth
    ui.Process(c, r.core);
    Check(Near(p.pitch, std::exp2(7.0 / 12.0), 1e-3),
          "S33: a stop at a fifth",
          p.pitch);
    c.knobs[Controls::S33] = 0.5f - 2.0f / 14.0f; // on the fourth down
    ui.Process(c, r.core);
    Check(Near(p.pitch, std::exp2(-5.0 / 12.0), 1e-3),
          "S33: a stop at a fourth down",
          p.pitch);
    c.knobs[Controls::S33] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.pitch, 4.0, 1e-3), "S33 top: +2 octaves", p.pitch);
    c.knobs[Controls::S33] = 0.5f;

    c.knobs[Controls::S32] = 1.0f;
    ui.Process(c, r.core);
    Check(p.window_shape == 1.0f && p.rand_shape == 0.0f,
          "S32 top: percussive, no randomness",
          p.window_shape);
    c.knobs[Controls::S32] = 0.5f;
    ui.Process(c, r.core);

    // The other second layers, each picked up at its value
    c.touched[CoreUI::kShiftPad] = true;
    c.knobs[Controls::S32]       = 1.0f;
    ui.Process(c, r.core);
    Check(p.rand_shape == 1.0f && p.window_shape == 0.5f,
          "P11 + S32: shape randomness, shape stays",
          p.rand_shape);
    c.knobs[Controls::S30] = 1.0f;
    ui.Process(c, r.core);
    Check(p.rand_pos == 1.0f && p.start == 0.0f,
          "P11 + S30 up: position, an even spread, position stays",
          p.rand_pos);
    Check(p.spread == 1.0f, "randomness: pan widens with it", p.spread);
    c.knobs[Controls::S30] = 0.0f;
    ui.Process(c, r.core);
    Check(p.rand_pos == -1.0f, "P11 + S30 down: the odd jump", p.rand_pos);
    const float own        = p.density;
    c.knobs[Controls::S34] = 0.5f; // from its value, at the centre
    ui.Process(c, r.core);
    c.knobs[Controls::S34] = 0.0f;
    ui.Process(c, r.core);
    Check(p.rand_density == -1.0f && p.density == own && p.spread == 1.0f,
          "P11 + S34 down: random starts, density stays, pan at its widest",
          p.rand_density);
    for(auto k : {Controls::S30,
                  Controls::S31,
                  Controls::S32,
                  Controls::S33,
                  Controls::S34,
                  Controls::S35})
    {
        c.knobs[k] = 1.0f; // picked up wherever it was
        ui.Process(c, r.core);
        c.knobs[k] = 0.5f;
        ui.Process(c, r.core);
    }
    Check(p.rand_pos == 0.0f && p.rand_size == 0.0f && p.rand_shape == 0.0f
              && p.rand_pitch == 0.0f && p.rand_density == 0.0f
              && p.rand_length == 0.0f && p.spread == 0.0f,
          "P11 + S30 to S35 at the centre: no randomness",
          p.spread);
    c.touched[CoreUI::kShiftPad] = false;
    c.knobs[Controls::S30]       = 0.0f;
    c.knobs[Controls::S31]       = 0.5f;
    c.knobs[Controls::S32]       = 0.5f;
    c.knobs[Controls::S33]       = 0.5f;
    c.knobs[Controls::S35]       = 1.0f;
    ui.Process(c, r.core);

    // The LED glows with the grains
    c.knobs[Controls::S34] = 0.755f;
    uint32_t ms            = 0;
    auto     press         = [&](size_t pad, bool on) {
        c.touched[pad] = on;
        ui.Process(c, r.core);
    };
    ui.Process(c, r.core);
    r.Run(0.5, Silence);
    const float led = ui.Led(ms + 1000, r.core);
    Check(Near(led, 0.6 * 0.6 * 0.6, 0.01), "LED: dim glow with grains", led);

    // A slice pad: a trigger of its own, the playhead untouched. Pressure sets
    // its density.
    const float  density = p.density;
    const double phase   = r.core.PlayheadPhase();
    const size_t place   = CoreUI::kSlicePads[3];
    c.pressure[place]    = 0.3f;
    press(place, true);
    r.Run(0.001, Silence);
    Check(p.density == density && r.core.TriggerOn(3)
              && p.triggers[3].density > 0.0f && p.triggers[3].density < 10.0f,
          "slice touched lightly: a sparse trigger, the playhead as it was",
          p.triggers[3].density);
    Check(std::fabs(r.core.PlayheadPhase() - phase) < 0.01,
          "slice touched: the playhead goes on",
          r.core.PlayheadPhase());
    c.pressure[place] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.triggers[3].density, Core::kMaxDensity, 0.01),
          "slice pressed hard: the trigger at the most density",
          p.triggers[3].density);
    press(place, false);
    Check(!p.triggers[3].held
              && p.triggers[3].mode == Core::Trigger::Mode::Once,
          "S07 up, released: once, it goes on to its end",
          0);
    c.switches[Controls::S07] = Controls::Centre;
    ui.Process(c, r.core);
    Check(
        p.triggers[3].mode == Core::Trigger::Mode::Gate, "S07 centre: gate", 1);
    c.switches[Controls::S07] = Controls::Down;
    ui.Process(c, r.core);
    Check(p.triggers[3].mode == Core::Trigger::Mode::Loop, "S07 down: loop", 2);
    c.switches[Controls::S07] = Controls::Up;

    // P02: the playhead from the start of the selection
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
    Triggers();
    Selection();
    Randomness();
    Starts();
    Pan();
    Voices();
    Onsets();
    Mapping();
    std::printf("%s\n",
                failures ? "Some checks failed." : "All checks passed.");
    return failures ? 1 : 0;
}
