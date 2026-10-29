// Host tests: the core and the Simple Touch mapping on a computer, no board.
// Built and run by tests/run.sh (make test). Each check prints PASS or FAIL.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>
#include "core/svf.h"
#include "ui/core.ui.h"

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

// A core with the board's memory: an 8 s recording, a 9 s ring
struct Rig {
    std::vector<float> ring = std::vector<float>(9 * kRate);
    std::vector<float> memory[Core::kNumBuffers] = { std::vector<float>(8 * kRate), std::vector<float>(8 * kRate) };
    Core core;
    double time = 0.0; // seconds of input so far
    float peak = 0.0f;
    bool finite = true;

    Rig() {
        float* buffers[Core::kNumBuffers] = { memory[0].data(), memory[1].data() };
        core.Init(kRate, ring.data(), ring.size(), buffers, 8 * kRate);
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
            core.Process(in, out, kBlock);
            for (size_t i = 0; i < kBlock; i++) {
                finite = finite && std::isfinite(o1[i]) && std::isfinite(o2[i]);
                peak = std::max(peak, std::max(std::fabs(o1[i]), std::fabs(o2[i])));
            }
        }
    }

    void Hold(double seconds, const std::function<float(double)>& input) {
        core.SetRecording(true);
        Run(seconds, input);
        core.SetRecording(false);
        Run(0.6, input); // the copy lands
    }

    double Seconds() const { return core.RecordedSeconds(); }
};

float Sine(double t) { return 0.5f * float(std::sin(2.0 * M_PI * 220.0 * t)); }
float Silence(double) { return 0.0f; }

void Capture() {
    Rig r;
    // 2 s silence, then sound: a tap keeps the 8 s before it, without the silence
    const double begin = 2.0;
    auto later = [begin](double t) { return t < begin ? 0.0f : Sine(t); };
    r.Run(4.0, later);
    r.Hold(0.1, later);
    Check(Near(r.Seconds(), 2.0, 0.03), "tap: the sound before it, silence cut (s)", r.Seconds());

    r.Run(1.0, Silence); // longer than the pre-roll
    r.Hold(1.0, Silence);
    Check(Near(r.Seconds(), 2.0, 0.03), "hold over silence: recording unchanged (s)", r.Seconds());

    r.Hold(15.0, Sine);
    Check(Near(r.Seconds(), 8.0, 0.001), "hold 15 s: the last 8 s (s)", r.Seconds());

    r.Hold(0.1, Sine);
    r.Hold(0.1, Sine);
    Check(Near(r.Seconds(), 8.0, 0.001), "two taps close together: 8 s each (s)", r.Seconds());

    // Trim: 5 ms kept before the first sound and after the last
    Rig t;
    auto burst = [](double x) { return x >= 0.5 && x < 1.5 ? Sine(x) : 0.0f; };
    t.Hold(3.0, burst);
    const Buffer& rec = t.core.Recording();
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
    // Noise recorded, then played at density 8: the normaliser keeps the input's level
    Rig r;
    uint32_t seed = 1;
    auto noise = [&seed](double) {
        seed = seed * 1664525u + 1013904223u;
        return 0.3f * (float(seed >> 8) / float(1 << 24) * 2.0f - 1.0f);
    };
    r.Hold(2.0, noise);
    Core::Params p;
    p.mix = 1.0f;
    p.density = 8.0f;
    r.core.SetParams(p);
    double in = 0.0, out = 0.0;
    float l[kBlock], o1[kBlock], o2[kBlock];
    const float* ins[2] = { l, l };
    float* outs[2] = { o1, o2 };
    for (size_t b = 0; b < 4000; b++) {
        for (size_t i = 0; i < kBlock; i++) l[i] = noise(0.0);
        r.core.Process(ins, outs, kBlock);
        if (b < 1000) continue;
        for (size_t i = 0; i < kBlock; i++) {
            in += double(l[i]) * l[i];
            out += 0.5 * (double(o1[i]) * o1[i] + double(o2[i]) * o2[i]);
        }
    }
    const double db = 10.0 * std::log10(out / in);
    Check(db > -2.0 && db < 0.5, "density 8: grains at the input's level (dB)", db);

    // Heavy: 20 ms grains, density 16, all places, reverb: never above the limiter
    p.grain_ms = 20.0f;
    p.density = 16.0f;
    p.place_density.fill(16.0f);
    p.reverb = 0.8f;
    p.rand_pitch = 12.0f;
    r.core.SetParams(p);
    r.peak = 0.0f;
    r.Run(3.0, Sine);
    Check(r.peak <= Limiter::kThreshold + 1e-4f, "heavy: output peak at most -0.5 dBFS", r.peak);
}

// One grain of 1000 samples over a recording of ones: its window, sample by sample
std::vector<float> Window(float shape) {
    std::vector<float> ones(kRate, 1.0f);
    Buffer buffer;
    buffer.Init(ones.data(), ones.size(), float(kRate));
    std::fill(ones.begin(), ones.end(), 1.0f);
    buffer.Finish(ones.size());
    Grain::InitWindow();
    Grain grain;
    Grain::Latch latch;
    latch.buffer = &buffer;
    latch.position = kRate / 2;
    latch.rate = 0.0f;
    latch.pan = 0.0f; // all left, gain 1
    latch.shape = shape;
    grain.Start(latch, 0);
    std::vector<float> window(1000, 0.0f), right(1000, 0.0f);
    Grain::Sums sums;
    grain.Render(1.0 / 1000.0, window.data(), right.data(), window.size(), sums);
    return window;
}

void Windows() {
    const auto hann = Window(0.5f);
    Check(Near(hann[250], 0.5, 0.01) && Near(hann[500], 1.0, 0.01), "window 0.5: Hann (at 1/4, 1/2)", hann[250]);
    const auto percussive = Window(1.0f);
    const size_t peak = size_t(std::max_element(percussive.begin(), percussive.end()) - percussive.begin());
    Check(peak == 10 && Near(percussive[500], 0.060, 0.005), "window 1: percussive (peak at 1 %, half way)", percussive[500]);
    const auto reverse = Window(0.0f);
    Check(Near(reverse[999 - 500], percussive[500], 0.002), "window 0: percussive reversed", reverse[499]);
}

// Share of the output's power within 10 Hz of the partials of 55 Hz, from 40 Hz to 1 kHz
double PartialShare(float harmonic) {
    Rig r;
    r.Hold(1.0, Sine); // 220 Hz: the 4th partial
    Core::Params p;
    p.mix = 1.0f;
    p.freeze = true;
    p.rand_pitch = 12.0f;
    p.harmonic = harmonic;
    r.core.SetParams(p);
    std::vector<float> mono;
    float l[kBlock], o1[kBlock], o2[kBlock];
    const float* ins[2] = { l, l };
    float* outs[2] = { o1, o2 };
    std::fill(l, l + kBlock, 0.0f);
    for (size_t b = 0; b < 2000; b++) {
        r.core.Process(ins, outs, kBlock);
        if (b < 1000) continue;
        for (size_t i = 0; i < kBlock; i++) mono.push_back(o1[i] + o2[i]);
    }
    double near = 0.0, total = 0.0;
    for (int hz = 40; hz <= 1000; hz++) {
        // Goertzel
        const double w = 2.0 * M_PI * hz / kRate, k = 2.0 * std::cos(w);
        double s1 = 0.0, s2 = 0.0;
        for (float x : mono) {
            const double s0 = x + k * s1 - s2;
            s2 = s1;
            s1 = s0;
        }
        const double power = s1 * s1 + s2 * s2 - k * s1 * s2;
        total += power;
        const double partial = std::round(hz / 55.0) * 55.0;
        if (std::fabs(hz - partial) <= 10.0) near += power;
    }
    return near / total;
}

// Grains at once on average over 20 s of a 1 s recording
double MeanActive(float size_variation) {
    Rig r;
    r.Hold(1.0, Sine);
    Core::Params p;
    p.density = 8.0f;
    p.jitter = 1.0f;
    p.size_variation = size_variation;
    r.core.SetParams(p);
    double active = 0.0;
    size_t blocks = 0;
    r.Run(20.0, Sine, [&](size_t) {
        for (const auto& g : r.core.Grains()) active += g.IsActive() ? 1.0 : 0.0;
        blocks++;
    });
    return active / double(blocks);
}

void SizeVariation() {
    const double fixed = MeanActive(0.0f), varied = MeanActive(1.0f);
    Check(Near(varied, fixed, 0.03 * fixed), "size variation: density kept (grains at once)", varied);
}

void Harmonic() {
    const double free = PartialShare(0.0f), snapped = PartialShare(1.0f);
    Check(free < 0.7, "random pitch, free: power near partials (share)", free);
    Check(snapped > 0.9, "random pitch, harmonic 1: power near partials (share)", snapped);
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
    Check(Near(Gain(4000, 200, 1.5f, true), -52.0, 1.0), "tone: lowpass 200 Hz at 4 kHz (dB)", Gain(4000, 200, 1.5f, true));
    Check(Near(Gain(200, 2000, 1.5f, false), -40.0, 1.0), "tone: highpass 2 kHz at 200 Hz (dB)", Gain(200, 2000, 1.5f, false));
    Check(std::fabs(Gain(10000, 20000, 0.707f, true)) < 0.05, "tone: off (lowpass 20 kHz) at 10 kHz (dB)",
          Gain(10000, 20000, 0.707f, true));
}

void Mapping() {
    Rig r;
    CoreUI ui;
    Controls c;
    c.knobs.fill(0.5f);
    c.knobs[Controls::S30] = 0.0f;
    c.knobs[Controls::S34] = 1.0f;
    c.knobs[Controls::S35] = 1.0f;
    c.knobs[Controls::S37] = 0.0f;
    c.switches = { Controls::Up, Controls::Up };
    ui.Process(c, r.core);
    const Core::Params& p = r.core.CurrentParams();
    Check(Near(p.grain_ms, 100.0, 0.01), "S31 centre: grain size (ms)", p.grain_ms);
    Check(Near(p.pitch, 1.0, 1e-6), "S33 centre: pitch (ratio)", p.pitch);
    Check(Near(p.speed, 1.0, 1e-6), "S36 centre: speed", p.speed);
    Check(Near(p.rand_pitch, 24.0, 1e-4), "S34 top: random pitch (semitones)", p.rand_pitch);
    Check(p.density == 0.0f, "S37 bottom: playhead density off", p.density);
    Check(Near(p.tone, 0.0, 1e-6), "P11 + S35 default: tone off", p.tone);

    // P10 held through the UI records; the LED glows with the grains
    c.knobs[Controls::S37] = 0.8f;
    uint32_t ms = 0;
    auto press = [&](size_t pad, bool on) {
        c.touched[pad] = on;
        ui.Process(c, r.core);
    };
    r.Run(1.5, Sine, [&](size_t b) {
        c.now_ms = ms = uint32_t(b);
        press(CoreUI::kRecordPad, b >= 500 && b < 1000); // 1 ms blocks
    });
    Check(Near(r.Seconds(), 0.55, 0.01), "P10 held 0.5 s: recording, with pre-roll (s)", r.Seconds());
    r.Run(0.5, Sine);
    const float led = ui.Led(ms + 1000, r.core);
    Check(Near(led, 0.6 * 0.6 * 0.6, 0.01), "LED: dim glow with grains", led);

    // P11 held and S35 turned: the second layer changes, the main layer doesn't
    const float tone_before = ui.SecondLayer(Controls::S35);
    press(CoreUI::kShiftPad, true);
    for (int i = 1; i <= 20; i++) {
        c.knobs[Controls::S35] = 1.0f - 0.02f * float(i);
        ui.Process(c, r.core);
    }
    press(CoreUI::kShiftPad, false);
    Check(ui.SecondLayer(Controls::S35) < tone_before - 0.2f, "P11 + S35: tone turned down", ui.SecondLayer(Controls::S35));
    Check(Near(r.core.CurrentParams().length, 1.0, 1e-6), "P11 + S35: selection end unchanged", r.core.CurrentParams().length);

    // Grain size ends; a place pressed half way
    c.knobs[Controls::S31] = 0.0f;
    ui.Process(c, r.core);
    Check(Near(r.core.CurrentParams().grain_ms, 10.0, 0.01), "S31 bottom: grain size (ms)", r.core.CurrentParams().grain_ms);
    c.knobs[Controls::S31] = 1.0f;
    ui.Process(c, r.core);
    Check(Near(r.core.CurrentParams().grain_ms, 500.0 * r.Seconds(), 0.1), "S31 top: half the recording (ms)", r.core.CurrentParams().grain_ms);
    const float density = r.core.CurrentParams().density;
    const size_t place = CoreUI::kPlacePads[0];
    c.pressure[place] = 0.5f;
    press(place, true);
    const Core::Params& pressed = r.core.CurrentParams();
    Check(pressed.place_density[0] == density && Near(pressed.level, std::sqrt(0.5), 1e-4) && Near(pressed.place_level[0], std::sqrt(0.5), 1e-4),
          "place pressed half: its density, equal-power crossfade", pressed.level);
    press(CoreUI::kStopPad, true);
    Check(r.core.CurrentParams().place_density[0] == 0.0f, "P02 held: places stopped", r.core.CurrentParams().place_density[0]);
    press(CoreUI::kStopPad, false);

    press(place, false);

    // P00 held and S37 down: output down; the main layer's density unchanged
    const float density_before = r.core.CurrentParams().density;
    press(CoreUI::kLevelsPad, true);
    for (int i = 1; i <= 40; i++) {
        c.knobs[Controls::S37] = 0.8f - 0.02f * float(i);
        ui.Process(c, r.core);
    }
    const float output = r.core.CurrentParams().output;
    press(CoreUI::kLevelsPad, false);
    Check(output < 0.01f && Near(r.core.CurrentParams().density, density_before, 1e-4), "P00 + S37 down: output off, density kept", output);

    // Detent pulse: on reaching the centre, renewed while turned within it
    c.knobs[Controls::S33] = 0.4f;
    c.now_ms = 10000;
    ui.Process(c, r.core);
    c.knobs[Controls::S33] = 0.49f;
    ui.Process(c, r.core);
    const float reached = ui.Led(10000 + CoreUI::kPulseMs - 1, r.core);
    c.now_ms = 10200;
    c.knobs[Controls::S33] = 0.51f;
    ui.Process(c, r.core);
    const float renewed = ui.Led(10200 + CoreUI::kPulseMs - 1, r.core);
    const float ended = ui.Led(10200 + CoreUI::kPulseMs, r.core);
    Check(reached == 1.0f && renewed == 1.0f && ended < 1.0f, "LED: pulse at pitch unison, renewed while turned", renewed);

    // P01 tapped: defaults, then the knob takes over from there
    c.knobs[Controls::S33] = 0.9f;
    ui.Process(c, r.core);
    const float pitch_before = r.core.CurrentParams().pitch;
    press(CoreUI::kResetPad, true);
    press(CoreUI::kResetPad, false);
    const float pitch_reset = r.core.CurrentParams().pitch;
    c.knobs[Controls::S33] = 0.95f;
    ui.Process(c, r.core);
    const float pitch_after = r.core.CurrentParams().pitch;
    Check(pitch_before > 1.5f && pitch_reset == 1.0f && pitch_after > 1.0f && pitch_after < pitch_before,
          "P01: pitch back to unison, knob takes over", pitch_after);
    Check(r.core.CurrentParams().output < 0.01f, "P01: output level kept", r.core.CurrentParams().output);
}

} // namespace

int main() {
    Capture();
    Level();
    Windows();
    Harmonic();
    SizeVariation();
    Tone();
    Mapping();
    std::printf("%s\n", failures ? "Some checks failed." : "All checks passed.");
    return failures ? 1 : 0;
}
