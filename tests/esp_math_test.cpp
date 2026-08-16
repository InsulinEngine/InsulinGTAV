// Host unit tests for the ESP geometry.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/esp_math_test.cpp -o build/esp_math_test.exe
//   ./build/esp_math_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "menu/base/util/esp_math.h"
#include <stdio.h>
#include <math.h>

static int g_failed = 0;

static void check_near(const char* what, float got, float want) {
    if (fabsf(got - want) > 0.0005f) { printf("FAIL %s: got %f, want %f\n", what, got, want); g_failed++; }
    else                             { printf("ok   %s = %f\n", what, got); }
}

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

int main() {
    using namespace menu::esp_math;

    // A ped whose head projects to y=0.4 and feet to y=0.6 is 0.2 tall on
    // screen, and the box is a quarter of that wide - 2take1's ratio.
    box2d b = box_from_projections(0.5f, 0.4f, 0.6f);
    check_near("height", b.h, 0.2f);
    check_near("width", b.w, 0.05f);
    check_near("left", b.x, 0.475f);      // centred on the head x
    check_near("top", b.y, 0.4f);         // the smaller of the two y

    // Upside down input (head below feet) must still give a positive box.
    box2d flipped = box_from_projections(0.5f, 0.6f, 0.4f);
    check_near("flipped height", flipped.h, 0.2f);
    check_near("flipped top", flipped.y, 0.4f);
    check_true("flipped width positive", flipped.w > 0.f);

    // Health bar: full health and full armour fills it; nothing empties it.
    // A ped-shaped case throughout - armour_max is the 50 GTA V allows a ped.
    check_near("full", health_fraction(100, 100, 50, 50), 1.0f);
    check_near("empty", health_fraction(0, 100, 0, 50), 0.0f);
    check_near("half health no armour", health_fraction(75, 100, 0, 50), 0.5f);

    // Health above max, or a zero max, must not escape [0,1] or divide by zero.
    check_near("over-full clamps", health_fraction(500, 100, 500, 50), 1.0f);
    check_near("zero max is empty", health_fraction(50, 0, 0, 50), 0.0f);
    check_near("negative is empty", health_fraction(-20, 100, 0, 50), 0.0f);

    // Non-ped path: no armour ceiling at all, so max_health alone is the
    // denominator. 100/(100+0) = 1.0, 50/(100+0) = 0.5.
    check_near("non-ped full", health_fraction(100, 100, 0, 0), 1.0f);
    check_near("non-ped half", health_fraction(50, 100, 0, 0), 0.5f);

    // A negative armour_max must clamp to 0, not shrink the denominator below
    // max_health. If it were left unclamped, 50/(100-10) = 0.5556 instead of
    // the 0.5 an un-armoured entity should read.
    check_near("negative armour_max clamps to 0", health_fraction(50, 100, 0, -10), 0.5f);

    // Text shrinks with distance, between the two given bounds, and never
    // outside them however far away the target is.
    check_near("nearest", distance_scale(0.f, 500.f, 0.25f, 0.15f), 0.25f);
    check_near("farthest", distance_scale(500.f, 500.f, 0.25f, 0.15f), 0.15f);
    check_near("halfway", distance_scale(250.f, 500.f, 0.25f, 0.15f), 0.20f);
    check_near("beyond clamps", distance_scale(9999.f, 500.f, 0.25f, 0.15f), 0.15f);
    check_near("zero range", distance_scale(10.f, 0.f, 0.25f, 0.15f), 0.15f);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
