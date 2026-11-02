#!/bin/sh
# Builds the web page into build/web: the site, the panel photo and
# clessidra.wasm (the core and mapping). Needs Emscripten. From the repo: make web
set -e
cd "$(dirname "$0")/.."

DAISYSP=lib/DaisySP
OUT=build/web
rm -rf "$OUT"
mkdir -p "$OUT"
cp -R web/site/. "$OUT"
cp touch.jpeg "$OUT/assets/"
em++ -O3 -std=c++17 -fno-exceptions -fno-rtti -Isrc \
    -I"$DAISYSP/Source" -I"$DAISYSP/Source/Utility" -I"$DAISYSP/DaisySP-LGPL/Source" \
    web/clessidra_web.cpp src/ui/*.cpp src/core/*.cpp \
    "$DAISYSP/Source/Utility/dcblock.cpp" "$DAISYSP/DaisySP-LGPL/Source/Effects/reverbsc.cpp" \
    --no-entry -sSTANDALONE_WASM -sINITIAL_MEMORY=16777216 -sALLOW_MEMORY_GROWTH=0 \
    -o "$OUT/clessidra.wasm"
ls -l "$OUT/clessidra.wasm"
