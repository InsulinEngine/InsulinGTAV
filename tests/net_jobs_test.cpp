// Host unit tests for the companion server's job ring.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/net_jobs_test.cpp src/net/jobs.cpp -o build/net_jobs_test.exe
//   ./build/net_jobs_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "net/jobs.h"
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

static net::job tp(float x) {
    net::job j;
    j.kind = net::job_kind::teleport;
    j.x = x; j.y = 0.0f; j.z = 0.0f; j.ground = true;
    return j;
}

int main() {
    net::job_ring r;
    r.reset();
    net::job out;

    check_true("empty pops nothing", !r.pop(&out));

    check_true("push accepted", r.push(tp(1.0f)));
    check_true("pop returns it", r.pop(&out));
    check_true("pop kind", out.kind == net::job_kind::teleport);
    check_true("pop payload", out.x == 1.0f);
    check_true("empty again", !r.pop(&out));

    // FIFO order, not LIFO.
    r.push(tp(1.0f)); r.push(tp(2.0f)); r.push(tp(3.0f));
    r.pop(&out); check_true("fifo first", out.x == 1.0f);
    r.pop(&out); check_true("fifo second", out.x == 2.0f);
    r.pop(&out); check_true("fifo third", out.x == 3.0f);

    // Filling to capacity works; one more is dropped, not written over a live slot.
    r.reset();
    for (unsigned i = 0; i < net::job_ring::capacity; i++) {
        check_true("fill accepted", r.push(tp((float)i)));
    }
    check_true("overflow refused", !r.push(tp(999.0f)));
    check_eq("dropped counted", r.dropped(), 1);

    // The oldest entry survived the refused push.
    check_true("oldest intact", r.pop(&out) && out.x == 0.0f);

    // Draining frees slots again - wrap-around is correct.
    while (r.pop(&out)) {}
    for (unsigned i = 0; i < net::job_ring::capacity; i++) {
        check_true("refill accepted", r.push(tp(100.0f + i)));
    }
    check_true("first after wrap", r.pop(&out) && out.x == 100.0f);

    // reset() clears the drop counter too.
    r.reset();
    check_eq("dropped cleared by reset", r.dropped(), 0);
    check_true("empty after reset", !r.pop(&out));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
