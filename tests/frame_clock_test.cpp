// Host unit tests for the animated-texture timing core.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/frame_clock_test.cpp -o build/frame_clock_test.exe
//   ./build/frame_clock_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "menu/base/util/frame_clock.h"
#include <stdio.h>
#include <stdint.h>

static int g_failed = 0;

static void check(const char* what, int got, int want) {
    if (got != want) { printf("FAIL %s: got %d, want %d\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %d\n", what, got); }
}

int main() {
    using namespace menu::frame_clock;

    // A three-frame sequence: 100 ms, 100 ms, 200 ms -> 400 ms per pass.
    const uint16_t d[3] = { 100, 100, 200 };
    check("total", total_ms(d, 3), 400);

    // Boundaries: a frame owns [start, start + delay).
    check("t=0",   frame_at(d, 3, 0,   true), 0);
    check("t=99",  frame_at(d, 3, 99,  true), 0);
    check("t=100", frame_at(d, 3, 100, true), 1);
    check("t=199", frame_at(d, 3, 199, true), 1);
    check("t=200", frame_at(d, 3, 200, true), 2);
    check("t=399", frame_at(d, 3, 399, true), 2);

    // Looping wraps modulo the total; a long gap must not run off the end.
    check("wrap t=400",    frame_at(d, 3, 400,    true), 0);
    check("wrap t=450",    frame_at(d, 3, 450,    true), 0);
    check("wrap t=500",    frame_at(d, 3, 500,    true), 1);
    check("wrap t=100000", frame_at(d, 3, 100000, true), 0);

    // One-shot stops on the last frame and stays there.
    check("oneshot t=400",   frame_at(d, 3, 400,   false), 2);
    check("oneshot t=99999", frame_at(d, 3, 99999, false), 2);

    // Degenerate inputs must not divide by zero, loop forever, or go out of range.
    const uint16_t zero[2] = { 0, 0 };
    check("all-zero delays", frame_at(zero, 2, 500, true), 0);
    check("zero total",      total_ms(zero, 2), 0);
    check("empty count",     frame_at(d, 0, 500, true), 0);
    check("negative elapsed", frame_at(d, 3, -5, true), 0);

    const uint16_t one[1] = { 66 };
    check("single frame t=0",    frame_at(one, 1, 0,    true), 0);
    check("single frame t=1000", frame_at(one, 1, 1000, true), 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
