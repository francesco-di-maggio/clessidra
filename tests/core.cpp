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
        c.knobs[Controls::S32] = 0.0f;
        c.knobs[Controls::S34] = 1.0f;
        c.knobs[Controls::S35] = 1.0f;
        c.knobs[Controls::S36] = 0.75f;
        c.knobs[Controls::S37] = 0.75f;
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

    // S34: dry passes the input as it is, wet only the grains
    auto  level = [](double) { return 0.25f; };
    Panel dry;
    dry.c.knobs[Controls::S34] = 0.0f;
    dry.Run(0.5, level);
    const double through = PanelRms(dry, 0.5, level);
    Check(Near(through, 0.25, 0.001),
          "S34 down: the input, dry, at its level (rms)",
          through);
    Panel wet;
    wet.Run(0.5, Sine);
    const double none = PanelRms(wet, 0.5, Sine);
    Check(none < 1e-6, "S34 up, nothing recorded: no dry signal (rms)", none);

    // A new recording: the LED flashes while held, the playhead restarts
    Panel led;
    led.c.touched[CoreUI::kRecordPad] = true;
    led.Run(0.5, Sine);
    const float on  = led.ui.Led(0, led.rig.core);
    const float off = led.ui.Led(CoreUI::kFlashMs, led.rig.core);
    Check(on == 1.0f && off == 0.0f, "recording: the LED flashes", off);

    // Recording again while the last one plays: no jump at the swap
    Panel again;
    again.c.knobs[Controls::S37]    = 1.0f;
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
    panel.c.knobs[Controls::S34]    = 1.0f;
    panel.c.knobs[Controls::S31]    = 0.5f;
    panel.c.knobs[Controls::S32]    = 0.0f;
    panel.c.knobs[Controls::S33]    = 0.5f;
    panel.c.knobs[Controls::S35]    = 1.0f;
    panel.c.knobs[Controls::S36]    = 0.75f;
    panel.c.knobs[Controls::S37]    = 0.75f;
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
    none.slice_start = none.slice_end = 0.5f;
    zero.core.SetParams(none);
    zero.Run(0.5, Sine);
    Check(zero.finite, "a slice without width: output finite", 1.0);
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
    c.knobs[Controls::S30] = 0.8f;
    c.knobs[Controls::S35] = 0.2f;
    ui.Process(c, r.core);
    Check(Near(p.start, 0.2, 1e-6) && Near(p.length, 0.6, 1e-6),
          "S30, S35: the selection between them, in either order",
          p.length);
    c.knobs[Controls::S35] = 0.8f;
    ui.Process(c, r.core);
    Check(Near(p.length, CoreUI::kMinLength, 1e-6) && p.start <= 0.8f,
          "S30 = S35: the shortest selection",
          p.length);
    c.knobs[Controls::S30] = 0.0f;
    c.knobs[Controls::S35] = 1.0f;
    ui.Process(c, r.core);
    Check(p.end == Playhead::End::OneShot, "S07 up: one-shot", int(p.end));

    c.knobs[Controls::S37] = 0.1f;
    ui.Process(c, r.core);
    const Core::Params low = p;
    c.knobs[Controls::S37] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(p.density, Core::kMaxDensity, 0.01) && low.density < 1.0f,
          "S37: grains per second, the most at the top",
          p.density);
    Check(p.grain_ms == low.grain_ms && p.window_shape == low.window_shape
              && p.reverb == low.reverb && p.jitter == low.jitter,
          "S37: density alone",
          p.grain_ms);
    c.knobs[Controls::S37] = 0.5f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, 200.0, 0.5), "S31 centre: 200 ms", p.grain_ms);
    c.knobs[Controls::S31] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(p.grain_ms, CoreUI::kMinGrainMs, 1e-3),
          "S31 bottom: 10 ms",
          p.grain_ms);
    c.knobs[Controls::S31] = 0.5f;
    ui.Process(c, r.core);

    // P11: the second layer, each knob picked up where its value is
    Check(p.window_shape == CoreUI::kShapeStart
              && p.reverb == CoreUI::kReverbStart && p.rand_pitch == 0.0f,
          "second layer at power-up: Hann, some reverb, plain pitch",
          p.reverb);
    c.touched[CoreUI::kLayerPad] = true;
    c.knobs[Controls::S34]       = 0.9f; // far from the reverb's value
    ui.Process(c, r.core);
    Check(p.reverb == CoreUI::kReverbStart && Near(p.mix, 0.5, 1e-6),
          "P11 + S34 away from the reverb: nothing moves",
          p.reverb);
    c.knobs[Controls::S34] = 0.2f; // across it
    ui.Process(c, r.core);
    c.knobs[Controls::S34] = 0.6f;
    ui.Process(c, r.core);
    Check(Near(p.reverb, 0.6, 1e-6) && Near(p.mix, 0.5, 1e-6),
          "P11 + S34 picked up: reverb, dry/wet stays",
          p.reverb);
    c.knobs[Controls::S31] = 0.0f; // across Hann
    ui.Process(c, r.core);
    c.knobs[Controls::S31] = 1.0f;
    ui.Process(c, r.core);
    Check(p.window_shape == 1.0f && Near(p.grain_ms, 200.0, 0.5),
          "P11 + S31: shape, size stays",
          p.window_shape);
    c.knobs[Controls::S33] = 0.0f; // at its value
    ui.Process(c, r.core);
    c.knobs[Controls::S33] = 1.0f;
    ui.Process(c, r.core);
    Check(p.rand_pitch == 1.0f && p.pitch == 1.0f,
          "P11 + S33: pitch scatter, pitch stays",
          p.rand_pitch);
    const float shown = ui.Led(0, r.core);
    Check(shown == 1.0f, "P11 held: the LED shows the knob last turned", shown);
    c.touched[CoreUI::kLayerPad] = false;
    ui.Process(c, r.core);
    Check(p.pitch == 1.0f && Near(p.mix, 0.5, 1e-6),
          "P11 released, knobs away: first layer unchanged",
          p.pitch);
    c.knobs[Controls::S31] = 0.5f;
    c.knobs[Controls::S33] = 0.5f;
    c.knobs[Controls::S34] = 0.5f;
    ui.Process(c, r.core);

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
    Check(p.jitter == 0.5f && p.size_variation == 0.5f && p.spread == 0.5f,
          "S32 centre: starts, sizes and pan half random",
          p.jitter);
    c.knobs[Controls::S32] = 1.0f;
    ui.Process(c, r.core);
    Check(
        p.rand_pitch == 1.0f, "S32: pitch scatter stays its own", p.rand_pitch);
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
    Record();
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
