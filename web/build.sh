#!/bin/sh
# Builds docs/clessidra.wasm: the engine and the Simple Touch mapping for the web page.
# Needs Emscripten (brew install emscripten). Run from anywhere.
set -e
cd "$(dirname "$0")/.."

DAISYSP=lib/DaisySP
em++ -O3 -std=c++17 -fno-exceptions -fno-rtti \
    -I"$DAISYSP/Source" -I"$DAISYSP/Source/Utility" -I"$DAISYSP/DaisySP-LGPL/Source" \
    web/clessidra_web.cpp ui/clessidra_ui.cpp \
    grain/engine.cpp grain/grain.cpp grain/buffer.cpp grain/limiter.cpp \
    "$DAISYSP/Source/Utility/dcblock.cpp" "$DAISYSP/DaisySP-LGPL/Source/Effects/reverbsc.cpp" \
    --no-entry -sSTANDALONE_WASM -sINITIAL_MEMORY=33554432 -sALLOW_MEMORY_GROWTH=0 \
    -o docs/clessidra.wasm

ls -l docs/clessidra.wasm
