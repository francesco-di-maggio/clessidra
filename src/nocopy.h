#pragma once

// Deletes copy and move: for objects that own hardware or large buffers
#define NOCOPY(T)                       \
    T(const T&)            = delete;    \
    T(T&&)                 = delete;    \
    T& operator=(const T&) = delete;    \
    T& operator=(T&&)      = delete;
