// Host unit tests for the script-event learn table.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_learn_test.cpp -o build/protections_learn_test.exe
//   ./build/protections_learn_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/learn.h"
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

    learn_table t;
    check_eq("starts empty", (unsigned)t.count(), 0);

    // A new hash is reported as new; a repeat is not. This is what lets the
    // hook report only the first sighting instead of flooding the ring.
    check_true("first sighting is new", t.observe(0xDEADBEEF, 3));
    check_true("second sighting is not new", !t.observe(0xDEADBEEF, 7));
    check_eq("still one distinct hash", (unsigned)t.count(), 1);

    uint32_t hash = 0, hits = 0; uint8_t first = 0xFF;
    check_true("readable", t.at(0, &hash, &hits, &first));
    check_eq("hash preserved", hash, 0xDEADBEEF);
    check_eq("hits counted", hits, 2);
    check_eq("first player is the FIRST one, not the latest", first, 3);

    // Distinct hashes stay distinct - the whole point.
    check_true("second hash is new", t.observe(0x11111111, 1));
    check_true("third hash is new", t.observe(0x22222222, 2));
    check_eq("three distinct", (unsigned)t.count(), 3);

    // Hash 0 is a legal event id and must be storable, not mistaken for an
    // empty slot.
    check_true("zero hash is new", t.observe(0, 4));
    check_eq("zero hash stored", (unsigned)t.count(), 4);
    check_true("zero hash is not new twice", !t.observe(0, 4));
    check_eq("still four", (unsigned)t.count(), 4);

    // Fill to capacity, then overflow. Overflow must be counted and must not
    // corrupt what is already there.
    t.clear();
    check_eq("cleared", (unsigned)t.count(), 0);
    check_eq("overflow cleared", t.overflow(), 0);

    for (uint32_t i = 0; i < learn_table::capacity; i++)
        check_true("fills without overflow", t.observe(0x1000 + i, 0));
    check_eq("full", (unsigned)t.count(), learn_table::capacity);
    check_eq("no overflow yet", t.overflow(), 0);

    t.observe(0xFFFFFFFF, 0);
    check_eq("overflow counted", t.overflow(), 1);
    check_eq("count did not grow past capacity", (unsigned)t.count(), learn_table::capacity);

    // An already-known hash must still register a hit when the table is full -
    // otherwise a full table stops counting the events you care about most.
    check_true("known hash still not new when full", !t.observe(0x1000, 0));
    check_eq("and did not count as overflow", t.overflow(), 1);

    // Out-of-range reads are inert.
    check_true("negative index reads nothing", !t.at(-1, &hash, &hits, &first));
    check_true("past-end index reads nothing", !t.at(t.count(), &hash, &hits, &first));
    check_true("null out is refused", !t.at(0, nullptr, &hits, &first));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
