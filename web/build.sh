#!/bin/sh
# Builds web/public/clessidra.wasm: the core and the mapping, for the web page.
# Needs Emscripten. From the repo: make wasm
set -e
cd "$(dirname "$0")/.."

DAISYSP=lib/DaisySP
em++ -O3 -std=c++17 -fno-exceptions -fno-rtti -Isrc \
    -I"$DAISYSP/Source" -I"$DAISYSP/Source/Utility" -I"$DAISYSP/DaisySP-LGPL/Source" \
    web/clessidra_web.cpp src/ui/*.cpp src/core/*.cpp \
    "$DAISYSP/Source/Utility/dcblock.cpp" "$DAISYSP/DaisySP-LGPL/Source/Effects/reverbsc.cpp" \
    --no-entry -sSTANDALONE_WASM -sINITIAL_MEMORY=33554432 -sALLOW_MEMORY_GROWTH=0 \
    -o web/public/clessidra.wasm
