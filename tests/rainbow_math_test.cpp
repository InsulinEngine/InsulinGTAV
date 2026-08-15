// Host unit tests for the rainbow stepping maths.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/rainbow_math_test.cpp -o build/rainbow_math_test.exe
//   ./build/rainbow_math_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "menu/base/util/rainbow_math.h"
#include <stdio.h>

static int g_failed = 0;

static void check(const char* what, int got, int want) {
    if (got != want) { printf("FAIL %s: got %d, want %d\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %d\n", what, got); }
}

static void check_range(const char* what, int got, int lo, int hi) {
    if (got < lo || got > hi) { printf("FAIL %s: %d outside [%d,%d]\n", what, got, lo, hi); g_failed++; }
    else                      { printf("ok   %s = %d in [%d,%d]\n", what, got, lo, hi); }
}

int main() {
    using namespace menu::rainbow_math;

    // A full cycle returns to where it started.
    rgb first = color_at(0, 80, 25, 250);
    rgb wrapped = color_at(80, 80, 25, 250);
    check("wrap r", wrapped.r, first.r);
    check("wrap g", wrapped.g, first.g);
    check("wrap b", wrapped.b, first.b);

    // Every channel at every step stays inside [min,max]: the cycle must never
    // reach black (invisible text) or full blast.
    for (int s = 0; s < 80; s++) {
        rgb c = color_at(s, 80, 25, 250);
        check_range("r", c.r, 25, 250);
        check_range("g", c.g, 25, 250);
        check_range("b", c.b, 25, 250);
    }

    // Degenerate inputs must not divide by zero or escape the range.
    rgb zero = color_at(5, 0, 25, 250);
    check_range("steps=0 r", zero.r, 25, 250);
    rgb one = color_at(5, 1, 25, 250);
    check_range("steps=1 r", one.r, 25, 250);

    // An inverted range is accepted rather than producing nonsense.
    rgb inverted = color_at(10, 80, 250, 25);
    check_range("inverted r", inverted.r, 25, 250);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
