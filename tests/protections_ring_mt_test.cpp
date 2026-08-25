// Host unit test: exercise the report ring under concurrent producers.
//
// This lives in its own file, separate from protections_ring_test.cpp,
// because it needs <thread>/<atomic>/<vector> - C++ headers - and the
// installed clang (18.1.8) has no libc++ on this box: `-stdlib=libc++` is
// silently ignored for the x86_64-pc-windows-msvc target here (no
// libc++.a/libc++.lib ships with this LLVM install; `clang++
// -stdlib=libc++ -print-file-name=libc++.a` just echoes the name back
// unresolved). So this file falls through to the real MSVC STL instead,
// which needs two things the plain `-I src ...` line used by the rest of
// the suite does not:
//
//   1. clang's MSVC auto-detection is broken on this box and silently
//      resolves to a stale "Microsoft Visual Studio 10.0" include path
//      instead of the installed VS2022 toolset, so INCLUDE/LIB must be set
//      explicitly before invoking clang.
//   2. MSVC STL 14.44 hard-refuses Clang < 19.0.0 with a static_assert
//      (error STL1000 - this is the same STL1000 the original suite's
//      C-headers-only rule was written to dodge). Clang 18.1.8 is one
//      major version short of that gate. -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH
//      is Microsoft's own documented escape hatch for exactly this
//      situation. -fms-compatibility-version must also be bumped to a real
//      VS2022 version, or char16_t/char32_t stop being recognized as
//      keywords (clang's un-detected fallback compat version, 16.0,
//      predates them being builtin types under /permissive- semantics).
//
// Build + run (from repo root; PowerShell shown, adjust for bash):
//   $env:INCLUDE = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um"
//   $env:LIB     = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64"
//   clang++ -std=c++17 -fms-compatibility-version=19.44.35207 -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH -I src tests/protections_ring_mt_test.cpp -o build/protections_ring_mt_test.exe
//   ./build/protections_ring_mt_test.exe
//
// If a given box has different MSVC/Windows-Kits versions installed, adjust
// the paths above to match; `dir "C:\Program Files\Microsoft Visual
// Studio\2022\Community\VC\Tools\MSVC"` and `dir "C:\Program Files
// (x86)\Windows Kits\10\Include"` list what is actually on disk.
//
// This is a host-only test binary. It links the real MSVC CRT/STL and is
// not part of the PS4 plugin build in any way - ring.h itself stays
// <stdint.h>-only and is not touched by any of the above.
//
// --- What this test does and does not verify (fix round 1) -----------------
//
// A review did adversarial mutation testing: three scratch copies of ring.h,
// each with exactly one property of the acquire/release protocol broken,
// built against this test and run repeatedly. Results below are this
// project's own re-run of that same exercise against the *current*
// (identity-tracking) version of this test, not carried over from the
// review - the mutant ring.h copies live under this session's scratchpad,
// never under src/, and are not part of the repo.
//
//   mutation                                   | catches (this test build)
//   --------------------------------------------|---------------------------
//   push(): CAS on m_tail -> plain store        | 30/30 at -O0, 30/30 at -O2
//     (racy claim - two producers can compute
//     the same slot index)
//   push(): m_ready release store -> relaxed    |  0/100 at -O0, 0/100 at -O2
//   pop():  m_ready never cleared (stale-ready) | 45/100 at -O0, 48/100 at -O2
//
// What this DOES verify, empirically, not just by hand-reasoning: the claim
// protocol itself - that concurrent producers never lose a claimed slot,
// never have two producers believe they own the same slot, and never leave
// a slot stuck unreadable. The racy-claim mutation is caught every time,
// and by two independent signals: the plain push/pop counters mismatch
// outright (a real run: "every push is accounted for: got 17928, want
// 20000" - a large UNDERcount, not an overcount; see the fixed explanation
// in task-2-report.md), AND the identity checks below additionally show a
// specific duplicated identity even inside that undercount. The stale-ready
// mutation, added in fix round 1, is now caught roughly half the time by
// the identity checks (0% before this round) - a genuine widening, but
// still a probabilistic one: it depends on a consumer reaching a reused
// slot in the narrow window before the new producer's write lands, which
// this test cannot force to happen on every run. A green run is evidence
// against stale-ready, not proof against it; run this test more than once
// when that matters (see the repeat-run counts in task-2-report.md).
//
// What this CANNOT verify on this host, and why: acquire/release ordering
// itself. The relaxed-release mutation - the actual ordering guarantee the
// protocol depends on, not just its consequences - was missed on every one
// of 200 runs (100 at -O0, 100 at -O2). This is not a weakness in the
// assertions that a cleverer check could fix: on x86-64, a release store
// and a relaxed store compile to the identical instruction (a plain MOV;
// x86-64's TSO-like memory model already provides store-release ordering
// for free), so there is no reordering for any observer on this
// architecture to ever witness, at any optimisation level, no matter how
// the assertions are written. The tool built for exactly this - detecting a
// missing ordering guarantee by tracking happens-before through shadow
// memory, rather than waiting for a reordering to become externally
// observable - is ThreadSanitizer, and `-fsanitize=thread` is rejected by
// clang for the x86_64-pc-windows-msvc target, so it is not available on
// this host today. Until this test runs on a target/toolchain combination
// where TSan (or an ARM/weak-memory host, where release-vs-relaxed is a
// real codegen difference) is available, a green run of this test is
// evidence about the claim protocol only. It says nothing about whether
// ring.h's memory_order arguments are individually correct - it happens to
// pass today even when one of them is silently wrong. Treat the ordering
// annotations in ring.h as verified by the Tier 1 hand review, not by this
// test.
// -----------------------------------------------------------------------
#include "protections/ring.h"
#include <stdio.h>
#include <thread>
#include <atomic>
#include <vector>

using namespace protections;

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

static void check_eq(const char* what, unsigned got, unsigned want) {
    if (got != want) { printf("FAIL %s: got %u, want %u\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %u\n", what, got); }
}

// Concurrent producers against a single consumer. The ring's whole reason for
// existing is that a network thread can push without blocking, and until now
// that has only ever been reasoned about, never executed.
static void test_concurrent_producers() {
    ring r;
    const int producers = 4;
    const int per_producer = 5000;

    std::atomic<int> pushed{0};
    std::atomic<bool> go{false};

    // seen[p][i] counts how many times pop() delivered the identity
    // (filter_id=p, detail_a=i). Fix-round-1 finding: the counter-only
    // assertions below are nearly tautological with one consumer (nothing
    // races pop() to inflate popped past the true count) and a double
    // delivery that exactly offsets a lost record passes them anyway, since
    // the totals still balance. Tracking identity closes that: it can tell
    // "lost + duplicated" apart from "everything delivered once".
    std::atomic<uint8_t> seen[producers][per_producer] = {};
    std::atomic<int> out_of_range{0};

    std::vector<std::thread> ts;
    for (int p = 0; p < producers; p++) {
        ts.emplace_back([&, p] {
            while (!go.load(std::memory_order_acquire)) {}
            for (int i = 0; i < per_producer; i++) {
                record rec = { (uint16_t)p, 0, 0, (uint32_t)i, 0 };
                r.push(rec);
                pushed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Records a popped record's identity. Anything outside the valid range
    // (filter_id >= producers or detail_a >= per_producer) is itself a
    // corruption - e.g. a torn read of a slot that raced a concurrent
    // write - and is counted separately rather than indexing out of bounds.
    auto record_seen = [&](const record& out) {
        if (out.filter_id < (unsigned)producers && out.detail_a < (unsigned)per_producer) {
            seen[out.filter_id][out.detail_a].fetch_add(1, std::memory_order_relaxed);
        } else {
            out_of_range.fetch_add(1, std::memory_order_relaxed);
        }
    };

    // One consumer, draining as fast as it can while the producers run.
    std::atomic<int> popped{0};
    std::thread consumer([&] {
        while (!go.load(std::memory_order_acquire)) {}
        record out;
        int spins = 0;
        while (popped.load(std::memory_order_relaxed) + (int)r.dropped()
               < producers * per_producer && spins < 100000000) {
            if (r.pop(&out)) { record_seen(out); popped.fetch_add(1, std::memory_order_relaxed); spins = 0; }
            else             { spins++; }
        }
    });

    go.store(true, std::memory_order_release);
    for (auto& t : ts) t.join();
    consumer.join();

    record out;
    while (r.pop(&out)) { record_seen(out); popped.fetch_add(1, std::memory_order_relaxed); }

    // Every push is either delivered or counted as dropped. Nothing vanishes,
    // and nothing is delivered twice. (Counter-only: see the header comment
    // for why these two alone are weak, and why the identity checks below
    // were added in fix round 1.)
    check_eq("every push is accounted for",
             (unsigned)(popped.load() + (int)r.dropped()),
             (unsigned)(producers * per_producer));
    check_true("nothing was delivered twice", popped.load() <= producers * per_producer);

    // Identity-level checks: walk every possible (producer, sequence)
    // identity that could ever have been pushed and confirm each one was
    // delivered at most once, and that the set never delivered is exactly
    // the set the ring itself says it dropped.
    check_eq("no popped record had a corrupt identity", (unsigned)out_of_range.load(), 0);

    unsigned seen_once = 0, seen_more_than_once = 0, seen_zero = 0;
    for (int p = 0; p < producers; p++) {
        for (int i = 0; i < per_producer; i++) {
            uint8_t c = seen[p][i].load(std::memory_order_relaxed);
            if (c == 0)      seen_zero++;
            else if (c == 1) seen_once++;
            else             seen_more_than_once++;
        }
    }
    check_eq("no identity was delivered more than once", seen_more_than_once, 0);
    check_eq("every never-delivered identity is exactly one the ring counted as dropped",
             seen_zero, (unsigned)r.dropped());
    check_eq("delivered-once identities match the popped count", seen_once, (unsigned)popped.load());
}

int main() {
    test_concurrent_producers();

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
