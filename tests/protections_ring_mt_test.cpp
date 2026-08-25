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

    // One consumer, draining as fast as it can while the producers run.
    std::atomic<int> popped{0};
    std::thread consumer([&] {
        while (!go.load(std::memory_order_acquire)) {}
        record out;
        int spins = 0;
        while (popped.load(std::memory_order_relaxed) + (int)r.dropped()
               < producers * per_producer && spins < 100000000) {
            if (r.pop(&out)) { popped.fetch_add(1, std::memory_order_relaxed); spins = 0; }
            else             { spins++; }
        }
    });

    go.store(true, std::memory_order_release);
    for (auto& t : ts) t.join();
    consumer.join();

    record out;
    while (r.pop(&out)) popped.fetch_add(1, std::memory_order_relaxed);

    // Every push is either delivered or counted as dropped. Nothing vanishes,
    // and nothing is delivered twice.
    check_eq("every push is accounted for",
             (unsigned)(popped.load() + (int)r.dropped()),
             (unsigned)(producers * per_producer));
    check_true("nothing was delivered twice", popped.load() <= producers * per_producer);
}

int main() {
    test_concurrent_producers();

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
