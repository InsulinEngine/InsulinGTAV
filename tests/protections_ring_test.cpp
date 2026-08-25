// Host unit tests for the protections report ring.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_ring_test.cpp -o build/protections_ring_test.exe
//   ./build/protections_ring_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/ring.h"
#include <stdio.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

static void check_eq(const char* what, unsigned got, unsigned want) {
    if (got != want) { printf("FAIL %s: got %u, want %u\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %u\n", what, got); }
}

int main() {
    using namespace protections;

    ring r;
    record out;

    // Empty ring pops nothing.
    check_true("empty pop fails", !r.pop(&out));

    // One in, one out, fields intact.
    record a = { 7, 3, 1, 0xDEADBEEF, 42 };
    r.push(a);
    check_true("pop after push", r.pop(&out));
    check_eq("filter_id", out.filter_id, 7);
    check_eq("player_index", out.player_index, 3);
    check_eq("flags", out.flags, 1);
    check_eq("detail_a", out.detail_a, 0xDEADBEEF);
    check_eq("detail_b", out.detail_b, 42);
    check_true("ring is empty again", !r.pop(&out));

    // FIFO order across a wrap: fill, drain, refill past the wrap point.
    r.reset();
    for (unsigned i = 0; i < ring::capacity; i++) {
        record x = { (unsigned short)i, 0, 0, i, 0 };
        r.push(x);
    }
    check_eq("no drops when exactly full", r.dropped(), 0);

    // One more than capacity is dropped, not overwritten.
    record overflow = { 999, 0, 0, 999, 0 };
    r.push(overflow);
    check_eq("one drop past capacity", r.dropped(), 1);

    // The original contents survived the overflow attempt, in order.
    for (unsigned i = 0; i < ring::capacity; i++) {
        check_true("pop in fifo order", r.pop(&out) && out.detail_a == i);
    }
    check_true("drained", !r.pop(&out));

    // After draining, the ring accepts pushes again (wrap-around correctness).
    for (unsigned i = 0; i < ring::capacity; i++) {
        record x = { 1, 0, 0, 1000 + i, 0 };
        r.push(x);
    }
    check_true("first after wrap", r.pop(&out) && out.detail_a == 1000);

    // reset() clears the drop counter too.
    r.reset();
    check_eq("dropped cleared by reset", r.dropped(), 0);
    check_true("empty after reset", !r.pop(&out));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
