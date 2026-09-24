# touch/

C++ hardware library for Simple Touch (Daisy Seed): 12 capacitive pads with
continuous pressure, 8 analog inputs (knobs/faders), 2 toggle switches.

```cpp
#include "daisy_seed.h"
#include "touch/touch.h"

using namespace daisy;
using namespace synthux;

static DaisySeed hw;
static Touch     touch;

int main(void) {
    hw.Init(true);
    touch.Init(hw);
    ...
```

### Pads (P00–P11)

```cpp
// Continuous pressure: 0.0 to 1.0 (quadratic finger-pulp response)
float p0 = touch.pads()[0];
float p_mid = touch.pads().Pressure(6);

// State queries
bool is_pressed = touch.pads().IsTouched(3);
bool any_active = touch.pads().HasTouch();

// Callbacks
touch.pads().SetOnTouch([](uint16_t pad) {
    // Pad pressed (0..11)
});

touch.pads().SetOnRelease([](uint16_t pad) {
    // Pad released (0..11)
});
```

### Knobs & Faders (0.0 to 1.0)

```cpp
// Rotary potentiometers (S30–S35)
float k0 = touch.knobs().s30();
float k1 = touch.knobs().s31();
float k_idx = touch.knobs().Knob(2); // S32

// Linear faders (S36 & S37)
float fader_l = touch.knobs().LeftFader();  // S36
float fader_r = touch.knobs().RightFader(); // S37

// Direct index access (0..7)
float raw_val = touch.knobs()[0];
```

### 3-Position Toggle Switches

```cpp
// Position queries
bool is_up   = touch.switches().IsAUp();
bool is_down = touch.switches().IsADown();
bool is_mid  = touch.switches().IsACenter();

// Enum values: Switch3::POS_UP, POS_CENTER, POS_DOWN
int pos_a = touch.switches().A();
int pos_b = touch.switches().B();
```

### Soft-Takeover (`MValue`)

Handles multi-function controls (e.g. shift layers) by latching values and requiring physical pickup before updating:

```cpp
static MValue cutoff(0.5f);
static MValue volume(0.85f);

bool is_shift = touch.pads().IsTouched(10);
float raw_fader = touch.knobs().LeftFader();

if (is_shift) {
    float v = volume.Process(raw_fader, true);
    cutoff.Process(raw_fader, false);
} else {
    float c = cutoff.Process(raw_fader, true);
    volume.Process(raw_fader, false);
}
```

### Chord Latch (`Latch`)

Handles chord latching / sustain pedal behavior:

```cpp
static Latch<12> latch;

latch.SetOnNoteOn([](uint8_t note) { ... });
latch.SetOnNoteOff([](uint8_t note) { ... });

// Toggle latch on/off
latch.Toggle();
```
