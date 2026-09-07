// Host unit tests for the companion teleport state machine.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/teleport_fsm_test.cpp -o build/teleport_fsm_test.exe
//   ./build/teleport_fsm_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "game/teleport_fsm.h"
#include <stdio.h>
#include <math.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

static void check_near(const char* what, float got, float want) {
    if (fabsf(got - want) > 0.01f) {
        printf("FAIL %s: got %.3f, want %.3f\n", what, got, want); g_failed++;
    } else {
        printf("ok   %s = %.3f\n", what, got);
    }
}

// --- the fake engine -------------------------------------------------------

static int   g_fade_out_calls = 0, g_fade_in_calls = 0, g_move_calls = 0, g_stream_calls = 0;
static float g_last_move_x, g_last_move_y, g_last_move_z;
static bool  g_is_faded = false;      // what faded_out() reports
static bool  g_ground_ok = true;      // whether ground_z() answers
static float g_ground_value = 42.0f;

static void fake_fade_out()  { g_fade_out_calls++; }
static bool fake_faded_out() { return g_is_faded; }
static void fake_fade_in()   { g_fade_in_calls++; }
static void fake_move(float x, float y, float z) {
    g_move_calls++; g_last_move_x = x; g_last_move_y = y; g_last_move_z = z;
}
static void fake_stream(float, float, float) { g_stream_calls++; }
static bool fake_ground_z(float, float, float, float* out) {
    if (!g_ground_ok) return false;
    *out = g_ground_value;
    return true;
}

static game::tp::actions fakes() {
    game::tp::actions a;
    a.fade_out  = fake_fade_out;
    a.faded_out = fake_faded_out;
    a.fade_in   = fake_fade_in;
    a.move      = fake_move;
    a.stream    = fake_stream;
    a.ground_z  = fake_ground_z;
    return a;
}

static void reset() {
    g_fade_out_calls = g_fade_in_calls = g_move_calls = g_stream_calls = 0;
    g_is_faded = false; g_ground_ok = true; g_ground_value = 42.0f;
}

int main() {
    const game::tp::actions a = fakes();

    // A fresh machine is idle and does nothing when ticked.
    {
        reset();
        game::tp::machine m;
        check_true("fresh is idle", m.state() == game::tp::phase::idle);
        check_true("fresh is not busy", !m.busy());
        m.tick(a);
        check_true("idle tick calls nothing", g_fade_out_calls == 0 && g_move_calls == 0);
    }

    // submit() starts the fade and nothing else; the move waits for black.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 100.0f, 200.0f, 0.0f, true };
        check_true("submit accepted", m.submit(r));
        check_true("submit is busy", m.busy());
        m.tick(a);
        check_true("fade requested once", g_fade_out_calls == 1);
        check_true("no move before black", g_move_calls == 0);
        check_true("phase is fading_out", m.state() == game::tp::phase::fading_out);

        // Ticking while the screen is still fading must not re-request the fade.
        m.tick(a);
        check_true("fade not re-requested", g_fade_out_calls == 1);

        // Once black, the machine moves to the probe altitude and asks the
        // world to stream in there.
        g_is_faded = true;
        m.tick(a);
        check_true("moved once black", g_move_calls == 1);
        check_near("probe x", g_last_move_x, 100.0f);
        check_near("probe y", g_last_move_y, 200.0f);
        check_near("probe z", g_last_move_z, game::tp::probe_z);
        check_true("collision requested", g_stream_calls == 1);
        check_true("phase is streaming", m.state() == game::tp::phase::streaming);
    }

    // A second submit while busy is refused, so sixteen queued map clicks
    // cannot interleave into sixteen overlapping teleports.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 1.0f, 2.0f, 0.0f, true };
        check_true("first submit ok", m.submit(r));
        check_true("second submit refused", !m.submit(r));
    }

    // The full happy path: ground resolves, the player lands just above it.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = true;
        m.tick(a);                                   // fade seen, move to probe
        for (int i = 0; i < game::tp::stream_frames; i++) m.tick(a);
        check_true("phase is resolving", m.state() == game::tp::phase::resolving);

        g_ground_value = 55.0f;
        m.tick(a);
        check_true("moved twice", g_move_calls == 2);
        check_near("landed just above ground", g_last_move_z, 56.0f);
        check_true("faded back in", g_fade_in_calls == 1);
        check_true("back to idle", m.state() == game::tp::phase::idle);
        check_true("not busy again", !m.busy());
    }

    // ground == false trusts the z it was given and never calls ground_z.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 77.0f, false };
        m.submit(r);
        g_is_faded = true;
        m.tick(a);
        for (int i = 0; i < game::tp::stream_frames; i++) m.tick(a);
        m.tick(a);
        check_near("used the given z", g_last_move_z, 77.0f);
        check_true("faded back in", g_fade_in_calls == 1);
        check_true("idle", m.state() == game::tp::phase::idle);
    }

    // The failure that matters: ground never resolves. The machine must give
    // up and fade back in regardless, because a teleport that leaves the
    // screen black forever is worse than one that drops the player.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = true;
        m.tick(a);
        for (int i = 0; i < game::tp::stream_frames; i++) m.tick(a);

        g_ground_ok = false;
        for (int i = 0; i < game::tp::resolve_frames + 2; i++) m.tick(a);
        check_true("gave up and faded in", g_fade_in_calls == 1);
        check_true("idle after giving up", m.state() == game::tp::phase::idle);
        check_true("left the player at probe height to fall",
                   g_last_move_z == game::tp::probe_z);
        check_true("accepts work again", m.submit(r));
    }

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
