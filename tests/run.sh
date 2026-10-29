#!/bin/sh
# Runs the host tests: the core and mapping (C++). No board needed. From the repo: make test
set -e
cd "$(dirname "$0")/.."

DAISYSP=lib/DaisySP
mkdir -p build/tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc \
    -I"$DAISYSP/Source" -I"$DAISYSP/Source/Utility" -I"$DAISYSP/DaisySP-LGPL/Source" \
    tests/core.cpp src/common/*.cpp src/ui/*.cpp src/core/*.cpp \
    "$DAISYSP/Source/Utility/dcblock.cpp" "$DAISYSP/DaisySP-LGPL/Source/Effects/reverbsc.cpp" \
    -o build/tests/core 2>&1 | grep -v "DaisySP" || true
echo "== Core and mapping"
build/tests/core
