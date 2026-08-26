// Host unit tests for the in-game report history (recent_history.h).
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_recent_history_test.cpp -o build/protections_recent_history_test.exe
//   ./build/protections_recent_history_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
//
// What this guards: protections_log_menu rebuilds its rows only when the history
// changes. Its dirty-flag used to key on (count, newest-record-content). Once the
// buffer is full, two consecutive byte-identical drained records are invisible to
// that check even though the history has shifted and the true oldest record was
// evicted - the menu then shows a view stale by one position. recent_history
// carries a generation counter that increments on EVERY append so the menu can
// detect the shift regardless of record content. These tests pin that property,
// plus the newest-first wraparound arithmetic that was previously untested.
#include "protections/ring.h"            // for record
#include "protections/recent_history.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

static void check_eq(const char* what, unsigned got, unsigned want) {
    if (got != want) { printf("FAIL %s: got %u, want %u\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %u\n", what, got); }
}

using namespace protections;

static record rec(uint16_t id, uint32_t a, uint32_t b) {
    record r = { id, 0xFF, 0, a, b };
    return r;
}

static bool records_equal(const record& x, const record& y) {
    return x.filter_id == y.filter_id && x.player_index == y.player_index &&
           x.flags == y.flags && x.detail_a == y.detail_a && x.detail_b == y.detail_b;
}

int main() {
    // --- empty state -------------------------------------------------------
    {
        recent_history_t<4> h;
        check_eq("empty count", (unsigned)h.count(), 0);
        check_eq("empty generation", h.generation(), 0);
        record out; uint32_t sup = 123;
        check_true("empty at(0) fails", !h.at(0, &out, &sup));
    }

    // --- append increments count and generation ----------------------------
    {
        recent_history_t<4> h;
        h.append(rec(1, 0xA, 0xB), 0);
        check_eq("one count", (unsigned)h.count(), 1);
        check_eq("one generation", h.generation(), 1);

        record out; uint32_t sup = 0;
        check_true("at(0) present", h.at(0, &out, &sup));
        check_true("at(0) is the appended record", records_equal(out, rec(1, 0xA, 0xB)));
        check_true("at(1) fails with one entry", !h.at(1, &out, &sup));
    }

    // --- newest first, and suppressed travels with the record --------------
    {
        recent_history_t<4> h;
        h.append(rec(1, 0, 0), 7);
        h.append(rec(2, 0, 0), 9);
        record out; uint32_t sup = 0;
        h.at(0, &out, &sup);
        check_true("at(0) is the newer record", out.filter_id == 2);
        check_eq("at(0) suppressed", sup, 9);
        h.at(1, &out, &sup);
        check_true("at(1) is the older record", out.filter_id == 1);
        check_eq("at(1) suppressed", sup, 7);
    }

    // --- capacity saturates, generation keeps climbing, oldest is evicted ---
    {
        recent_history_t<4> h;
        for (uint16_t i = 0; i < 6; i++) h.append(rec(i, i, 0), 0);
        check_eq("count saturates at capacity", (unsigned)h.count(), 4);
        check_eq("generation counts every append", h.generation(), 6);

        record out; uint32_t sup = 0;
        h.at(0, &out, &sup);
        check_true("newest after wrap is the last appended", out.filter_id == 5);
        h.at(3, &out, &sup);
        check_true("oldest after wrap is index 2 (0 and 1 evicted)", out.filter_id == 2);
        check_true("out-of-range at(4) fails", !h.at(4, &out, &sup));
    }

    // --- THE FIX: two byte-identical appends at full capacity --------------
    // A dirty-check keyed on (count, newest-content) cannot see this shift;
    // the generation counter must. And the shift is real - the oldest changes.
    {
        recent_history_t<4> h;
        h.append(rec(10, 1, 1), 0);
        h.append(rec(11, 2, 2), 0);
        h.append(rec(12, 3, 3), 0);
        h.append(rec(13, 4, 4), 0);   // full: [13,12,11,10]

        const record x = rec(99, 0xDEAD, 0xBEEF);
        h.append(x, 0);               // [x,13,12,11], 10 evicted

        record newest0, oldest0; uint32_t s0 = 0;
        h.at(0, &newest0, &s0);
        h.at(3, &oldest0, &s0);
        const int      count0 = h.count();
        const uint32_t gen0   = h.generation();

        h.append(x, 0);               // identical content: [x,x,13,12], 11 evicted

        record newest1, oldest1; uint32_t s1 = 0;
        h.at(0, &newest1, &s1);
        h.at(3, &oldest1, &s1);
        const int      count1 = h.count();
        const uint32_t gen1   = h.generation();

        // A content-only dirty-check (the old menu logic) reports "unchanged":
        check_true("count unchanged across the collision", count0 == count1);
        check_true("newest content unchanged across the collision",
                   records_equal(newest0, newest1));

        // ...but the history genuinely shifted - the true oldest was evicted:
        check_true("oldest actually changed (view went stale by one)",
                   !records_equal(oldest0, oldest1));

        // ...and the generation counter is what makes the shift visible:
        check_true("generation changed across the collision", gen0 != gen1);
    }

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
