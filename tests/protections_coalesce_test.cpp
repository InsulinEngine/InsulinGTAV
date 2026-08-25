// Host unit tests for per-filter report coalescing.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_coalesce_test.cpp -o build/protections_coalesce_test.exe
//   ./build/protections_coalesce_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
//
// What this guards: one noisy filter must not be able to fill the 64-slot ring
// and starve every other filter's reports. task_ambient_clips fires once per
// null-anims-group ped per FSM tick and ships enabled, so this is the ordinary
// case, not an attack case.
#include "protections/ring.h"
#include "protections/coalesce.h"
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

using namespace protections;

// Mirrors the producer half of protections::report() exactly: count, claim,
// push only if the claim succeeded.
static unsigned g_total = 0;
static void emit(ring& rg, coalescer& c, unsigned short id, unsigned detail) {
    g_total++;
    if (!c.claim(id)) return;
    record r = { id, 0xFF, 0, detail, 0 };
    rg.push(r);
}

// Mirrors the consumer half of protections::drain_reports(): pop, release,
// report the suppressed count alongside. Returns how many records were drained
// and writes the per-record suppressed counts into `more`.
static unsigned drain(ring& rg, coalescer& c, record* recs, unsigned* more, unsigned cap) {
    unsigned n = 0;
    record r;
    for (unsigned guard = 0; guard < ring::capacity && n < cap && rg.pop(&r); guard++) {
        recs[n] = r;
        more[n] = c.release(r.filter_id);
        n++;
    }
    return n;
}

int main() {
    ring      rg;
    coalescer c;
    record    recs[ring::capacity];
    unsigned  more[ring::capacity];

    const unsigned short NOISY = 14;   // filter_id::task_ambient_clips
    const unsigned short OTHER = 15;   // filter_id::task_parachute

    // ---- 1. A burst from one filter collapses to a single record carrying the
    // count of everything it swallowed.
    const unsigned N = 100;            // more than the ring could ever hold
    for (unsigned i = 0; i < N; i++) emit(rg, c, NOISY, i);

    check_eq("burst pushed nothing beyond the first", rg.dropped(), 0);
    check_eq("suppressed count while queued", c.suppressed(NOISY), N - 1);
    check_true("filter is marked pending", c.pending(NOISY));

    unsigned n = drain(rg, c, recs, more, ring::capacity);
    check_eq("burst of N yields exactly one record", n, 1);
    check_eq("record is from the noisy filter", recs[0].filter_id, NOISY);
    check_eq("record carries the first occurrence", recs[0].detail_a, 0);
    check_eq("record reports N-1 suppressed", more[0], N - 1);
    check_true("filter reopened after drain", !c.pending(NOISY));
    check_eq("no occurrence went unaccounted", g_total, N);

    // ---- 2. THE STARVATION CASE. A second filter reporting in the middle of a
    // burst still gets its own record. Without coalescing the ring would be full
    // of NOISY by now and this record would be dropped.
    rg.reset(); c.reset(); g_total = 0;

    for (unsigned i = 0; i < 200; i++) {
        emit(rg, c, NOISY, i);
        if (i == 150) emit(rg, c, OTHER, 0xABCD);   // mid-burst
    }
    check_eq("no drops during a 201-event burst", rg.dropped(), 0);

    n = drain(rg, c, recs, more, ring::capacity);
    check_eq("two filters, two records", n, 2);

    bool saw_noisy = false, saw_other = false;
    for (unsigned i = 0; i < n; i++) {
        if (recs[i].filter_id == NOISY) { saw_noisy = true; check_eq("noisy suppressed", more[i], 199); }
        if (recs[i].filter_id == OTHER) {
            saw_other = true;
            check_eq("other filter kept its payload", recs[i].detail_a, 0xABCD);
            check_eq("other filter suppressed nothing", more[i], 0);
        }
    }
    check_true("noisy filter reported", saw_noisy);
    check_true("quiet filter was NOT starved", saw_other);

    // ---- 3. After a drain the filter can report again, and the counter starts
    // from zero rather than carrying the old burst forward.
    check_eq("counter cleared by release", c.suppressed(NOISY), 0);
    emit(rg, c, NOISY, 0x1234);
    n = drain(rg, c, recs, more, ring::capacity);
    check_eq("reports again after drain", n, 1);
    check_eq("fresh payload", recs[0].detail_a, 0x1234);
    check_eq("fresh record suppressed nothing", more[0], 0);

    // ---- 4. Ring usage is bounded by filter count, not event rate: every
    // filter can be mid-burst at once and the ring still never overflows.
    rg.reset(); c.reset(); g_total = 0;
    for (unsigned round = 0; round < 50; round++)
        for (unsigned short id = 0; id < 24; id++)
            emit(rg, c, id, round);
    check_eq("24 filters x 50 rounds, no drops", rg.dropped(), 0);
    n = drain(rg, c, recs, more, ring::capacity);
    check_eq("one record per distinct filter", n, 24);
    check_eq("total still counts every occasion", g_total, 50u * 24u);

    // ---- 5. An id past the ceiling fails safe: it is never coalesced, so it
    // behaves exactly as the ring did before this change.
    rg.reset(); c.reset();
    const unsigned short WILD = coalescer::id_ceiling;   // out of range
    for (unsigned i = 0; i < 3; i++) emit(rg, c, WILD, i);
    n = drain(rg, c, recs, more, ring::capacity);
    check_eq("out-of-range id is not coalesced", n, 3);
    check_eq("out-of-range release reports nothing", more[0], 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
