// Host unit tests for the camera rotation -> direction expansion.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/camera_dir_test.cpp -o build/camera_dir_test.exe
//   ./build/camera_dir_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "game/camera_dir_math.h"
#include <stdio.h>
#include <math.h>

static int g_failed = 0;

static void check_near(const char* what, float got, float want, float tol) {
    float d = got - want; if (d < 0.f) d = -d;
    if (d > tol) { printf("FAIL %s: got %.4f, want %.4f\n", what, got, want); g_failed++; }
    else         { printf("ok   %s = %.4f\n", what, got); }
}

static void dir(float pitch, float yaw, float* x, float* y, float* z) {
    game::camera_math::direction_from_rotation(pitch, yaw, x, y, z);
}

int main() {
    float x, y, z;

    // Yaw 0 is forward (+Y). This one passes even with the degree/radian bug,
    // which is exactly why it is not enough on its own.
    dir(0.f, 0.f, &x, &y, &z);
    check_near("yaw0 x", x, 0.f, 0.001f);
    check_near("yaw0 y", y, 1.f, 0.001f);
    check_near("yaw0 z", z, 0.f, 0.001f);

    // Yaw 90 must be a quarter turn to the left (-X). THIS is the assertion that
    // catches degrees being fed to a radian expansion or the reverse: with the
    // units mixed up, 90 collapses to roughly 1.57 and x comes out near -0.027
    // instead of -1.
    dir(0.f, 90.f, &x, &y, &z);
    check_near("yaw90 x", x, -1.f, 0.001f);
    check_near("yaw90 y", y,  0.f, 0.001f);

    dir(0.f, 180.f, &x, &y, &z);
    check_near("yaw180 y", y, -1.f, 0.001f);

    dir(0.f, 270.f, &x, &y, &z);
    check_near("yaw270 x", x, 1.f, 0.001f);

    // Straight up and straight down.
    dir(90.f, 0.f, &x, &y, &z);
    check_near("pitch90 z", z, 1.f, 0.001f);
    check_near("pitch90 y", y, 0.f, 0.001f);

    dir(-90.f, 0.f, &x, &y, &z);
    check_near("pitch-90 z", z, -1.f, 0.001f);

    // Always unit length. Note this holds even under the unit bug, so it proves
    // the expansion is well formed - not that the units are right.
    const float cases[][2] = { {0,0}, {30,45}, {-20,200}, {60,300}, {89,17} };
    for (int i = 0; i < 5; i++) {
        dir(cases[i][0], cases[i][1], &x, &y, &z);
        check_near("unit length", sqrtf(x*x + y*y + z*z), 1.f, 0.001f);
    }

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
