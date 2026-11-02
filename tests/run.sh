#!/bin/sh
# Runs the host tests: the core and mapping (C++). No board needed. From the repo: make test
set -e
cd "$(dirname "$0")/.."

DAISYSP=lib/DaisySP
mkdir -p build/tests
# A failed build stops here: never the tests of the last one
c++ -std=c++17 -O2 -Wall -Wextra -Isrc \
    -I"$DAISYSP/Source" -I"$DAISYSP/Source/Utility" -I"$DAISYSP/DaisySP-LGPL/Source" \
    tests/core.cpp src/ui/*.cpp src/core/*.cpp \
    "$DAISYSP/Source/Utility/dcblock.cpp" "$DAISYSP/DaisySP-LGPL/Source/Effects/reverbsc.cpp" \
    -o build/tests/core 2> build/tests/compile.log || { cat build/tests/compile.log; exit 1; }
grep -v "DaisySP" build/tests/compile.log || true
echo "== Core and mapping"
build/tests/core

# The web build, where Emscripten is installed
if command -v em++ > /dev/null; then
    sh web/build.sh
    echo "== Web"
    node tests/web.mjs
    npm --prefix web run build > /dev/null
    echo "== Play page"
    node tests/play.mjs
else
    echo "== Web: skipped, no em++"
fi
