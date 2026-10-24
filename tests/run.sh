#!/bin/sh
# Runs the host tests: the engine and mapping (C++). No board needed. From the repo: make test
set -e
cd "$(dirname "$0")/.."

DAISYSP=lib/DaisySP
mkdir -p build/tests
c++ -std=c++17 -O2 -Wall -Wextra -I. \
    -I"$DAISYSP/Source" -I"$DAISYSP/Source/Utility" -I"$DAISYSP/DaisySP-LGPL/Source" \
    tests/engine.cpp ui/clessidra_ui.cpp grain/engine.cpp grain/grain.cpp grain/buffer.cpp grain/limiter.cpp \
    "$DAISYSP/Source/Utility/dcblock.cpp" "$DAISYSP/DaisySP-LGPL/Source/Effects/reverbsc.cpp" \
    -o build/tests/engine 2>&1 | grep -v "DaisySP" || true
echo "== Engine and mapping"
build/tests/engine
