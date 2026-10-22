// Host tests: the engine and the Simple Touch mapping on a computer, no board.
// Built and run by tests/run.sh (make test). Each check prints PASS or FAIL.

#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>
#include "grain/svf.h"
#include "ui/clessidra_ui.h"

using namespace clessidra;

namespace {

constexpr size_t kRate = 48000;
constexpr size_t kBlock = 48;
int failures = 0;

void Check(bool ok, const char* name, double value) {
    std::printf("%s  %-52s %.4g\n", ok ? "PASS" : "FAIL", name, value);
    if (!ok) failures++;
}

bool Near(double value, double target, double tolerance) {
    return std::fabs(value - target) <= tolerance;
}

// An engine with the board's memory: a 10 s recording, an 11 s ring
struct Rig {
    std::vector<float> ring = std::vector<float>(11 * kRate);
    std::vector<float> memory[Engine::kNumBuffers] = { std::vector<float>(10 * kRate), std::vector<float>(10 * kRate) };
    Engine engine;
    double time = 0.0; // seconds of input so far
    float peak = 0.0f;
    bool finite = true;

    Rig() {
        float* buffers[Engine::kNumBuffers] = { memory[0].data(), memory[1].data() };
        engine.Init(kRate, ring.data(), ring.size(), buffers, 10 * kRate);
    }

    // Runs the input for seconds; between blocks, each(block)
    void Run(double seconds, const std::function<float(double)>& input,
             const std::function<void(size_t)>& each = nullptr) {
        float l[kBlock], o1[kBlock], o2[kBlock];
        const float* in[2] = { l, l };
        float* out[2] = { o1, o2 };
        const size_t blocks = size_t(seconds * kRate / kBlock);
        for (size_t b = 0; b < blocks; b++) {
            if (each) each(b);
            for (size_t i = 0; i < kBlock; i++) {
                l[i] = input(time);
                time += 1.0 / kRate;
            }
            engine.Process(in, out, kBlock);
            for (size_t i = 0; i < kBlock; i++) {
                finite = finite && std::isfinite(o1[i]) && std::isfinite(o2[i]);
                peak = std::max(peak, std::max(std::fabs(o1[i]), std::fabs(o2[i])));
            }
        }
    }

    void Hold(double seconds, const std::function<float(double)>& input) {
        engine.SetRecording(true);
        Run(seconds, input);
        engine.SetRecording(false);
        Run(0.6, input); // the copy lands
    }

    double Seconds() const { return engine.RecordedSeconds(); }
};

float Sine(double t) { return 0.5f * float(std::sin(2.0 * M_PI * 220.0 * t)); }
float Silence(double) { return 0.0f; }

void Capture() {
    Rig r;
    // 2 s silence, then sound: a tap keeps the 10 s before it, without the silence
    const double begin = 2.0;
    auto later = [begin](double t) { return t < begin ? 0.0f : Sine(t); };
    r.Run(4.0, later);
    r.Hold(0.1, later);
    Check(Near(r.Seconds(), 2.0, 0.03), "tap: the sound before it, silence cut (s)", r.Seconds());

    r.Run(1.0, Silence); // longer than the pre-roll
    r.Hold(1.0, Silence);
    Check(Near(r.Seconds(), 2.0, 0.03), "hold over silence: recording unchanged (s)", r.Seconds());

    r.Hold(15.0, Sine);
    Check(Near(r.Seconds(), 10.0, 0.001), "hold 15 s: the last 10 s (s)", r.Seconds());

    r.Hold(0.1, Sine);
    r.Hold(0.1, Sine);
    Check(Near(r.Seconds(), 10.0, 0.001), "two taps close together: 10 s each (s)", r.Seconds());

    // Trim: 5 ms kept before the first sound and after the last
    Rig t;
    auto burst = [](double x) { return x >= 0.5 && x < 1.5 ? Sine(x) : 0.0f; };
    t.Hold(3.0, burst);
    const Buffer& rec = t.engine.Recording();
    size_t first = rec.Length(), last = 0;
    for (size_t i = 0; i < rec.Length(); i++) {
        if (std::fabs(rec.Data()[i]) > Ring::kSilence) {
            if (first == rec.Length()) first = i;
            last = i;
        }
    }
    const double margin = Buffer::kFadeSeconds * kRate;
    Check(Near(double(first), margin, 1.0), "trim: samples kept before the sound", double(first));
    Check(Near(double(rec.Length() - 1 - last), margin, 1.0), "trim: samples kept after the sound", double(rec.Length() - 1 - last));
    Check(t.finite, "output finite throughout", 1.0);
}

void Level() {
    // Noise recorded, then played at flow 8: the normaliser keeps the input's level
    Rig r;
    uint32_t seed = 1;
    auto noise = [&seed](double) {
        seed = seed * 1664525u + 1013904223u;
        return 0.3f * (float(seed >> 8) / float(1 << 24) * 2.0f - 1.0f);
    };
    r.Hold(2.0, noise);
    Engine::Params p;
    p.mix = 1.0f;
    p.flow = 8.0f;
    r.engine.SetParams(p);
    double in = 0.0, out = 0.0;
    float l[kBlock], o1[kBlock], o2[kBlock];
    const float* ins[2] = { l, l };
    float* outs[2] = { o1, o2 };
    for (size_t b = 0; b < 4000; b++) {
        for (size_t i = 0; i < kBlock; i++) l[i] = noise(0.0);
        r.engine.Process(ins, outs, kBlock);
        if (b < 1000) continue;
        for (size_t i = 0; i < kBlock; i++) {
            in += double(l[i]) * l[i];
            out += 0.5 * (double(o1[i]) * o1[i] + double(o2[i]) * o2[i]);
        }
    }
    const double db = 10.0 * std::log10(out / in);
    Check(db > -2.0 && db < 0.5, "flow 8: grains at the input's level (dB)", db);

    // Heavy: 20 ms grains, flow 16, all places, reverb: never above the limiter
    p.grain_ms = 20.0f;
    p.flow = 16.0f;
    p.place_flow.fill(16.0f);
    p.reverb = 0.8f;
    p.rand_pitch = 12.0f;
    r.engine.SetParams(p);
    r.peak = 0.0f;
    r.Run(3.0, Sine);
    Check(r.peak <= Limiter::kThreshold + 1e-4f, "heavy: output peak at most -0.5 dBFS", r.peak);
}

double Gain(float hz, float cutoff, float q, bool low) {
    Svf f;
    f.Set(cutoff, q, float(kRate));
    double in = 0.0, out = 0.0;
    for (size_t n = 0; n < 2 * kRate; n++) {
        const float x = float(std::sin(2.0 * M_PI * hz * double(n) / kRate));
        f.Process(x);
        const float y = low ? f.Low() : f.High();
        if (n < kRate) continue;
        in += double(x) * x;
        out += double(y) * y;
    }
    return 10.0 * std::log10(out / in);
}

void Tone() {
    Check(Near(Gain(4000, 500, 1.5f, true), -36.4, 1.0), "tone: lowpass 500 Hz at 4 kHz (dB)", Gain(4000, 500, 1.5f, true));
    Check(Near(Gain(200, 2000, 1.5f, false), -40.0, 1.0), "tone: highpass 2 kHz at 200 Hz (dB)", Gain(200, 2000, 1.5f, false));
    Check(std::fabs(Gain(10000, 20000, 0.707f, true)) < 0.05, "tone: off (lowpass 20 kHz) at 10 kHz (dB)",
          Gain(10000, 20000, 0.707f, true));
}

void Mapping() {
    Rig r;
    ClessidraUI ui;
    Controls c;
    c.knobs.fill(0.5f);
    c.knobs[Controls::S30] = 0.0f;
    c.knobs[Controls::S34] = 1.0f;
    c.knobs[Controls::S35] = 1.0f;
    c.knobs[Controls::S37] = 0.0f;
    c.switches = { Controls::Up, Controls::Up };
    ui.Process(c, r.engine);
    const Engine::Params& p = r.engine.CurrentParams();
    Check(Near(p.grain_ms, 100.0, 0.01), "S31 centre: grain size (ms)", p.grain_ms);
    Check(Near(p.pitch, 1.0, 1e-6), "S33 centre: pitch (ratio)", p.pitch);
    Check(Near(p.speed, 1.0, 1e-6), "S36 centre: speed", p.speed);
    Check(Near(p.rand_pitch, 24.0, 1e-4), "S34 top: random pitch (semitones)", p.rand_pitch);
    Check(p.flow == 0.0f, "S37 bottom: playhead flow off", p.flow);
    Check(Near(p.tone, 0.0, 1e-6), "P11 + S35 default: tone off", p.tone);

    // P10 held through the UI records; the LED glows with the grains
    c.knobs[Controls::S37] = 0.8f;
    uint32_t ms = 0;
    auto press = [&](size_t pad, bool on) {
        c.touched[pad] = on;
        ui.Process(c, r.engine);
    };
    r.Run(1.5, Sine, [&](size_t b) {
        c.now_ms = ms = uint32_t(b);
        press(ClessidraUI::kRecordPad, b >= 500 && b < 1000); // 1 ms blocks
    });
    Check(Near(r.Seconds(), 0.55, 0.01), "P10 held 0.5 s: recording, with pre-roll (s)", r.Seconds());
    r.Run(0.5, Sine);
    const float led = ui.Led(ms + 1000, r.engine);
    Check(Near(led, 0.6 * 0.6 * 0.6, 0.01), "LED: dim glow with grains", led);

    // P11 held and S35 turned: the second layer changes, the main layer doesn't
    const float tone_before = ui.SecondLayer(Controls::S35);
    press(ClessidraUI::kShiftPad, true);
    for (int i = 1; i <= 20; i++) {
        c.knobs[Controls::S35] = 1.0f - 0.02f * float(i);
        ui.Process(c, r.engine);
    }
    press(ClessidraUI::kShiftPad, false);
    Check(ui.SecondLayer(Controls::S35) < tone_before - 0.2f, "P11 + S35: tone turned down", ui.SecondLayer(Controls::S35));
    Check(Near(r.engine.CurrentParams().length, 1.0, 1e-6), "P11 + S35: selection end unchanged", r.engine.CurrentParams().length);

    ui.SetSecondLayer(Controls::S34, 0.3f);
    Check(Near(ui.SecondLayer(Controls::S34), 0.3, 1e-6), "second layer set directly (web page)", ui.SecondLayer(Controls::S34));
}

} // namespace

int main() {
    Capture();
    Level();
    Tone();
    Mapping();
    std::printf("%s\n", failures ? "Some checks failed." : "All checks passed.");
    return failures ? 1 : 0;
}
