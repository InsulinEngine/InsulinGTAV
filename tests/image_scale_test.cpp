// Host unit test for the downscale target arithmetic.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/image_scale_test.cpp -o build/image_scale_test.exe
//   ./build/image_scale_test.exe
#include "util/image/scale.h"
#include <stdio.h>

static int g_failed = 0;

static void check2(const char* what, int gw, int gh, int ww, int wh) {
    if (gw != ww || gh != wh) {
        printf("FAIL %s: got %dx%d, want %dx%d\n", what, gw, gh, ww, wh); g_failed++;
    } else printf("ok   %s = %dx%d\n", what, gw, gh);
}

int main() {
    int w, h;

    // Smaller than the box is left alone - never upscale a small logo into mush.
    util::image::fit_within(64, 32, 512, 128, &w, &h);
    check2("smaller untouched", w, h, 64, 32);

    // Exactly the box is left alone.
    util::image::fit_within(512, 128, 512, 128, &w, &h);
    check2("exact fit untouched", w, h, 512, 128);

    // Too wide: width binds, aspect kept.
    util::image::fit_within(1024, 128, 512, 128, &w, &h);
    check2("width binds", w, h, 512, 64);

    // Too tall: height binds.
    util::image::fit_within(256, 2048, 512, 1024, &w, &h);
    check2("height binds", w, h, 128, 1024);

    // A 4K photo into the background box.
    util::image::fit_within(3840, 2160, 512, 1024, &w, &h);
    check2("4k into background", w, h, 512, 288);

    // Extreme ratios must never round a dimension to zero - a zero-sized
    // texture is a crash waiting in the DDS writer, not a small picture.
    util::image::fit_within(4000, 2, 512, 1024, &w, &h);
    if (w < 1 || h < 1) { printf("FAIL extreme ratio produced %dx%d\n", w, h); g_failed++; }
    else printf("ok   extreme ratio = %dx%d\n", w, h);

    util::image::fit_within(2, 4000, 512, 1024, &w, &h);
    if (w < 1 || h < 1) { printf("FAIL extreme ratio 2 produced %dx%d\n", w, h); g_failed++; }
    else printf("ok   extreme ratio 2 = %dx%d\n", w, h);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
