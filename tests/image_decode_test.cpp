// Host unit test for the image decoder.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src -I <toolchain>/include tests/image_decode_test.cpp src/util/image/decode.cpp -o build/image_decode_test.exe
//   ./build/image_decode_test.exe
// stb_image is C and compiles on the host; nothing here includes a PS4 header.
#include "util/image/decode.h"
#include <stdio.h>

static int g_failed = 0;

static void check(const char* what, int got, int want) {
    if (got != want) { printf("FAIL %s: got %d, want %d\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %d\n", what, got); }
}

static void check_true(const char* what, bool cond) {
    if (!cond) { printf("FAIL %s\n", what); g_failed++; }
    else       { printf("ok   %s\n", what); }
}

int main() {
    util::image::decoded d;

    // A real GIF from the repo. Asserting concrete numbers rather than "it
    // returned something" - a decoder that hands back one blank frame would pass
    // a truthiness check and fail this.
    check_true("decode assets/test.gif", util::image::decode_file("assets/test.gif", &d));
    check_true("width > 0",  d.w > 0);
    check_true("height > 0", d.h > 0);
    check_true("at least one frame", d.frames >= 1);
    check_true("pixels not null", d.rgba != nullptr);

    // Every frame needs a delay, and a zero delay would make an animation spin
    // at the frame rate.
    for (int i = 0; i < d.frames; i++)
        check_true("delay > 0", d.delays_ms[i] > 0);

    // Not every pixel identical: catches a decoder that returns a cleared buffer.
    bool varied = false;
    for (int i = 4; i < d.w * d.h * 4 && !varied; i += 4)
        if (d.rgba[i] != d.rgba[0]) varied = true;
    check_true("frame is not a flat fill", varied);

    util::image::free_decoded(&d);

    // A file that is not an image must fail cleanly, not crash or half-fill.
    util::image::decoded bad;
    check_true("garbage rejected", !util::image::decode_file("tests/image_decode_test.cpp", &bad));

    check_true("missing file rejected", !util::image::decode_file("does/not/exist.png", &bad));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
