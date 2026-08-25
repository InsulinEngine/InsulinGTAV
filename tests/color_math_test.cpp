// Host unit tests for the colour conversion the HSVA editor round-trips through.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/color_math_test.cpp -o build/color_math_test.exe
//   ./build/color_math_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "menu/base/util/color_math.h"
#include <stdio.h>

static int g_failed = 0;

static void check_near(const char* what, int got, int want, int tolerance) {
    int d = got - want; if (d < 0) d = -d;
    if (d > tolerance) { printf("FAIL %s: got %d, want %d (+-%d)\n", what, got, want, tolerance); g_failed++; }
    else               { printf("ok   %s = %d (want %d)\n", what, got, want); }
}

int main() {
    using namespace menu::color_math;

    // A round trip must land back where it started. One unit of rounding is
    // acceptable; anything more and repeated format switches walk the colour.
    const int samples[][3] = {
        { 0, 149, 255 }, { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 },
        { 255, 255, 255 }, { 0, 0, 0 }, { 128, 128, 128 }, { 34, 139, 34 },
        { 255, 214, 98 }, { 52, 49, 72 },
    };

    for (int i = 0; i < (int)(sizeof(samples) / sizeof(samples[0])); i++) {
        hsv h = rgb_to_hsv(samples[i][0], samples[i][1], samples[i][2]);
        int r = 0, g = 0, b = 0;
        hsv_to_rgb(h.h, h.s, h.v, &r, &g, &b);
        check_near("round trip r", r, samples[i][0], 1);
        check_near("round trip g", g, samples[i][1], 1);
        check_near("round trip b", b, samples[i][2], 1);
    }

    // Grey has no meaningful hue; saturation must be zero rather than garbage.
    hsv grey = rgb_to_hsv(128, 128, 128);
    check_near("grey saturation", (int)(grey.s * 100.f), 0, 0);

    // Out-of-range input must CLAMP, not wrap. These two cases are chosen so a
    // wrapping implementation gives a different answer and the test fails:
    // -90 clamped to 0 is red-ish, -90 wrapped to 270 would be violet.
    int r = 0, g = 0, b = 0;
    hsv_to_rgb(-90.f, 0.5f, 0.5f, &r, &g, &b);
    check_near("clamp low r", r, 128, 1);
    check_near("clamp low g", g, 64, 1);
    check_near("clamp low b", b, 64, 1);

    hsv_to_rgb(400.f, 2.f, 2.f, &r, &g, &b);   // h->360 (hue 0), s->1, v->1
    check_near("clamp high r", r, 255, 1);
    check_near("clamp high g", g, 0, 1);
    check_near("clamp high b", b, 0, 1);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
