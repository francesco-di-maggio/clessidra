# Clessidra

A Simple Touch (Daisy Seed) instrument inspired by the hourglass, in progress.

---

## 1. Hardware Layout

<img src="touch.jpeg" width="350"/>


---

## 2. Quickstart

```bash
# Clone with submodules
git clone --recurse-submodules -b dev https://github.com/francesco-di-maggio/clessidra.git
cd clessidra

# Build libraries, then TouchMIDI (the root Makefile's build target)
make libs -j4
make -j4

# Flash to Daisy Seed (hold BOOT, press RESET, release BOOT)
make program-dfu

# Open TouchMIDI/TouchMIDI.maxpat in Max to test against it
```

---

## 3. License

MIT
