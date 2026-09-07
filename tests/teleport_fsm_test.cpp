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

static void check_int(const char* what, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d, want %d\n", what, got, want); g_failed++;
    } else {
        printf("ok   %s = %d\n", what, got);
    }
}

// --- the fake engine -------------------------------------------------------

static int   g_fade_out_calls = 0, g_fade_in_calls = 0, g_move_calls = 0, g_stream_calls = 0;
static float g_last_move_x, g_last_move_y, g_last_move_z;
static bool  g_is_faded = false;      // what faded_out() reports
static bool  g_ground_ok = true;      // whether ground_z() answers at all
// The altitude at or below which ground_z() starts answering. The whole point
// of the sweep is that the query cannot answer from too high up, so the fake
// has to be able to model that rather than answering from anywhere.
static float g_ground_from = 1.0e9f;
static float g_ground_value = 42.0f;
static int   g_ground_calls = 0;
// Split by value rather than one running total, so a test can assert
// "true exactly once, false exactly once" directly instead of inferring it
// from a combined count plus the last value.
static int   g_freeze_true_calls = 0, g_freeze_false_calls = 0;
static bool  g_last_freeze = false;
// How many move() calls had happened when freeze(true) arrived. The header
// says the move must come first - freezing before it would pin the subject at
// its old position - so that ordering is pinned here rather than trusted.
static int   g_moves_at_freeze = -1;
// Every altitude move() was called with, in order, so a test can prove the
// sweep actually walked down the ladder instead of jumping to the answer.
static float g_move_z[64];
static int   g_move_n = 0;

static void fake_fade_out()  { g_fade_out_calls++; }
static bool fake_faded_out() { return g_is_faded; }
static void fake_fade_in()   { g_fade_in_calls++; }
static void fake_move(float x, float y, float z) {
    g_move_calls++; g_last_move_x = x; g_last_move_y = y; g_last_move_z = z;
    if (g_move_n < (int)(sizeof(g_move_z) / sizeof(g_move_z[0])))
        g_move_z[g_move_n++] = z;
}
static void fake_stream(float, float, float) { g_stream_calls++; }
static bool fake_ground_z(float, float, float probe, float* out) {
    g_ground_calls++;
    if (!g_ground_ok) return false;
    if (probe > g_ground_from) return false;
    *out = g_ground_value;
    return true;
}
static void fake_freeze(bool on) {
    if (on) { g_freeze_true_calls++; g_moves_at_freeze = g_move_calls; }
    else    { g_freeze_false_calls++; }
    g_last_freeze = on;
}

static game::tp::actions fakes() {
    game::tp::actions a;
    a.fade_out  = fake_fade_out;
    a.faded_out = fake_faded_out;
    a.fade_in   = fake_fade_in;
    a.move      = fake_move;
    a.stream    = fake_stream;
    a.ground_z  = fake_ground_z;
    a.freeze    = fake_freeze;
    return a;
}

static void reset() {
    g_fade_out_calls = g_fade_in_calls = g_move_calls = g_stream_calls = 0;
    g_is_faded = false; g_ground_ok = true; g_ground_value = 42.0f;
    g_ground_from = 1.0e9f; g_ground_calls = 0;
    g_freeze_true_calls = g_freeze_false_calls = 0;
    g_last_freeze = false; g_moves_at_freeze = -1;
    g_move_n = 0;
}

// Ticks until the flight ends, returning how many frames that took. The cap is
// a test-side backstop: if a phase ever loses its bounded exit (invariant 1),
// this returns the cap instead of hanging the test run forever.
static int run_until_idle(game::tp::machine& m, const game::tp::actions& a, int cap) {
    int n = 0;
    while (m.busy() && n < cap) { m.tick(a); n++; }
    return n;
}

int main() {
    const game::tp::actions a = fakes();
    const float top    = game::tp::rungs[0];
    const float bottom = game::tp::rungs[game::tp::rung_count - 1];

    // A fresh machine is idle and does nothing when ticked.
    {
        reset();
        game::tp::machine m;
        check_true("fresh is idle", m.state() == game::tp::phase::idle);
        check_true("fresh is not busy", !m.busy());
        check_true("fresh has no outcome", m.result() == game::tp::outcome::none);
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

        // Once black, the machine drops onto the top rung of the ladder and
        // asks the world to stream in there.
        g_is_faded = true;
        m.tick(a);
        check_true("moved once black", g_move_calls == 1);
        check_near("top rung x", g_last_move_x, 100.0f);
        check_near("top rung y", g_last_move_y, 200.0f);
        check_near("top rung z", g_last_move_z, top);
        check_true("collision requested", g_stream_calls == 1);
        check_true("phase is sweeping", m.state() == game::tp::phase::sweeping);

        // The ordering the header calls out as load-bearing: place, then hold.
        check_true("froze exactly once", g_freeze_true_calls == 1);
        check_int("moves before the freeze", g_moves_at_freeze, 1);
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

    // Ground answers on the very first rung: the sweep stops there and never
    // walks down. (Mount Chiliad's case - the summit is under the top rung.)
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = true;
        g_ground_value = 55.0f;
        m.tick(a);                         // fade seen, move to the top rung
        check_true("phase is sweeping", m.state() == game::tp::phase::sweeping);
        m.tick(a);                         // first query answers
        check_true("moved twice", g_move_calls == 2);
        check_near("landed just above ground", g_last_move_z, 56.0f);
        check_true("faded back in", g_fade_in_calls == 1);
        check_true("unfroze on the landing path", g_freeze_false_calls == 1);
        check_true("back to idle", m.state() == game::tp::phase::idle);
        check_true("not busy again", !m.busy());
        check_true("outcome is landed", m.result() == game::tp::outcome::landed);
        check_true("landed() latched", m.landed());
        check_int("rung that answered", m.rung(), 0);
        check_near("ground height reported", m.ground_height(), 55.0f);
    }

    // The case the sweep exists for: nothing answers from high up, and the
    // query only starts working once the subject is low enough. The machine
    // must walk down the ladder rung by rung and take the first hit.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = true;
        g_ground_value = 40.0f;
        g_ground_from  = 120.0f;   // only rungs at or below 120m can answer
        int frames = run_until_idle(m, a, 500);
        check_true("sweep finished inside its bound", frames < 500);
        check_true("outcome is landed", m.result() == game::tp::outcome::landed);
        check_int("rung that answered", m.rung(), 5);
        check_near("that rung is the first at or below the cutoff",
                   game::tp::rungs[m.rung()], 100.0f);
        check_near("landed just above ground", g_last_move_z, 41.0f);
        // one move onto the top rung, five steps down the ladder, one landing
        check_int("moves", g_move_calls, 7);
        check_true("walked the ladder in order",
                   g_move_n == 7 &&
                   g_move_z[0] == game::tp::rungs[0] &&
                   g_move_z[1] == game::tp::rungs[1] &&
                   g_move_z[2] == game::tp::rungs[2] &&
                   g_move_z[3] == game::tp::rungs[3] &&
                   g_move_z[4] == game::tp::rungs[4] &&
                   g_move_z[5] == game::tp::rungs[5]);
        check_true("froze once, unfroze once",
                   g_freeze_true_calls == 1 && g_freeze_false_calls == 1);
        check_true("faded back in", g_fade_in_calls == 1);
    }

    // The failure that matters: the ground never answers from any rung. The
    // machine must exhaust the ladder, lift the player back to a survivable
    // altitude, and hand the screen back - a teleport that leaves the screen
    // black forever is worse than one that drops the player a hundred metres.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = true;
        g_ground_ok = false;
        m.tick(a);                             // fade seen -> sweeping
        int frames = run_until_idle(m, a, 1000);

        // Invariant 1 with a number on it, and the answer to the review's
        // point about budgets: the build that failed acceptance spent 30
        // streaming frames + 60 resolving frames = 90 before giving up. The
        // sweep must not quietly spend less than that, or a failed run would
        // not distinguish "wrong approach" from "not enough time".
        check_int("frames spent sweeping before giving up", frames,
                  (game::tp::rung_count - 1) * game::tp::rung_frames +
                  game::tp::last_rung_frames);
        check_true("spends at least as long as the build that failed", frames >= 90);

        check_true("gave up and faded in", g_fade_in_calls == 1);
        check_true("idle after giving up", m.state() == game::tp::phase::idle);
        check_true("outcome is no_ground", m.result() == game::tp::outcome::no_ground);
        check_true("landed() is false", !m.landed());
        check_int("no rung answered", m.rung(), -1);
        check_near("left at the survivable fallback, not the bottom rung",
                   g_last_move_z, game::tp::fallback_z);
        check_true("fallback is well above the bottom rung",
                   game::tp::fallback_z > bottom);
        // one move onto the top rung, seven steps down, one lift to fallback
        check_int("moves", g_move_calls, 9);
        check_true("unfroze on the give-up path", g_freeze_false_calls == 1);
        check_true("accepts work again", m.submit(r));
    }

    // ground == false trusts the z it was given, never calls ground_z, and
    // still holds the subject there while collision loads under it.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 77.0f, false };
        m.submit(r);
        g_is_faded = true;
        m.tick(a);
        check_true("phase is holding", m.state() == game::tp::phase::holding);
        check_near("parked on the requested z", g_last_move_z, 77.0f);
        int frames = run_until_idle(m, a, 500);
        check_int("held for the full settle", frames, game::tp::hold_frames);
        check_near("used the given z", g_last_move_z, 77.0f);
        check_int("never asked for ground", g_ground_calls, 0);
        check_true("outcome is placed", m.result() == game::tp::outcome::placed);
        check_true("faded back in", g_fade_in_calls == 1);
        check_true("unfroze on the ground:false path", g_freeze_false_calls == 1);
        check_true("idle", m.state() == game::tp::phase::idle);
    }

    // Invariant 1 where it actually bit us: the screen never reports black,
    // because something else (a mission script, a wasted screen, an interior
    // transition) issued its own fade-in inside our window. Without a bound
    // the machine sits here forever with the fade out and busy() stuck true,
    // which silently kills every later teleport in the session.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = false;                  // never goes black
        int frames = run_until_idle(m, a, 100000);
        check_int("gave up on the fade after its bound", frames, game::tp::fade_frames);
        check_true("idle, not stuck busy", m.state() == game::tp::phase::idle);
        check_true("outcome is no_fade", m.result() == game::tp::outcome::no_fade);
        check_true("handed the screen back", g_fade_in_calls == 1);
        check_true("never moved the player", g_move_calls == 0);
        check_true("never froze anyone", g_freeze_true_calls == 0);
        check_true("released anyway, harmlessly", g_freeze_false_calls == 1);
        check_true("accepts work again", m.submit(r));
    }

    // Invariant 2, given its own block across every exit the machine has,
    // because it is the one thing here that must never regress - a player
    // left frozen in mid-air hangs there permanently, which is strictly worse
    // than the fall the freeze exists to prevent.
    {
        const char* names[4] = {
            "unfreezes: landed on the first rung",
            "unfreezes: landed partway down the ladder",
            "unfreezes: ladder exhausted",
            "unfreezes: ground:false"
        };
        for (int i = 0; i < 4; i++) {
            reset();
            game::tp::machine m;
            game::tp::request r = { 10.0f, 20.0f, 77.0f, i != 3 };
            m.submit(r);
            g_is_faded = true;
            if (i == 1) g_ground_from = 120.0f;
            if (i == 2) g_ground_ok = false;
            int frames = run_until_idle(m, a, 1000);
            check_true(names[i],
                       frames < 1000 &&
                       !m.busy() &&
                       g_freeze_false_calls == 1 &&
                       g_last_freeze == false &&
                       g_fade_in_calls == 1);
        }
        // And the one exit where nothing was ever frozen: still released, and
        // still not left busy.
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        int frames = run_until_idle(m, a, 1000);
        check_true("unfreezes: fade never went black",
                   frames < 1000 && !m.busy() && g_freeze_false_calls == 1);
    }

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
