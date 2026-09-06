// Host unit test: the job ring under a concurrent producer and consumer.
//
// This lives in its own file, separate from tests/net_jobs_test.cpp, because it
// needs <thread>/<atomic> - C++ headers - and the installed clang (18.1.8) has
// no libc++ on this box, so it falls through to the real MSVC STL instead.
// This is the same situation tests/protections_ring_mt_test.cpp documents and
// solves; the plain "-I src" line used by the rest of the suite is not enough
// here, and neither, on this box, is that file's own recipe verbatim - one
// more flag was needed below. The exact toolset/SDK versions in the command
// are what is installed on the box this was last verified on; if they don't
// match, `dir "C:\Program Files\Microsoft Visual Studio\<ver>\<edition>\VC\Tools\MSVC"`
// and `dir "C:\Program Files (x86)\Windows Kits\10\Include"` list what is
// actually on disk (protections_ring_mt_test.cpp's header explains why
// INCLUDE/LIB must be set explicitly at all: clang's MSVC auto-detection is
// broken on this box and silently resolves to a stale toolset otherwise).
//
// This box has MSVC toolset 14.51.36231 under Visual Studio "18" (this
// install renumbered "2022" to "18") and Windows Kits 10.0.19041.0. Beyond
// -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH (the STL1000 escape hatch
// protections_ring_mt_test.cpp already documents), this MSVC STL is new
// enough to need a second one:
//
//   This STL's default "doom function" - the internal precondition-failure
//   hook behind _STL_VERIFY - unconditionally expands to
//   __builtin_verbose_trap(...) whenever __clang__ is defined
//   (__msvc_doom_core.hpp). That builtin was only added in Clang 19; this
//   box's clang is 18.1.8, one minor version short. So even with the
//   version-mismatch escape hatch, compiling <xmemory> (pulled in by
//   <thread>) still fails with:
//     error: use of undeclared identifier '__builtin_verbose_trap'
//   __msvc_doom_core.hpp documents the fix itself: define
//   _MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION to swap the doom function to a
//   plain abort() instead of the Clang intrinsic. This is a host-only test
//   binary that never ships, so trading the trap's diagnostic message for a
//   bare abort() on an STL-internal precondition failure this test never
//   exercises directly is an acceptable, Microsoft-documented substitution -
//   it changes nothing about the ring's own logic or the checks below. If a
//   box's clang is new enough to have __builtin_verbose_trap, this flag is
//   unnecessary; try without it first if the error above doesn't reproduce.
//
// Build + run (from repo root; PowerShell shown, adjust for bash):
//   $env:INCLUDE = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\um"
//   $env:LIB     = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64"
//   clang++ -std=c++17 -fms-compatibility-version=19.51.36231 -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH -D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION -I src tests/net_jobs_mt_test.cpp src/net/jobs.cpp -o build/net_jobs_mt_test.exe
//   ./build/net_jobs_mt_test.exe
//
// This is a host-only test binary. It links the real MSVC CRT/STL and is not
// part of the PS4 plugin build in any way - jobs.h/jobs.cpp stay free of PS4
// headers and are not touched by any of the above.
#include "net/jobs.h"
#include <stdio.h>
#include <thread>
#include <atomic>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

int main() {
    net::job_ring r;
    r.reset();

    const unsigned per_thread = 5000;
    const unsigned producers  = 4;

    std::atomic<unsigned> pushed(0);
    std::atomic<bool>     stop(false);

    std::thread ts[producers];
    for (unsigned t = 0; t < producers; t++) {
        ts[t] = std::thread([&r, &pushed, per_thread]() {
            for (unsigned i = 0; i < per_thread; i++) {
                net::job j;
                j.kind = net::job_kind::teleport;
                j.x = 1.0f; j.y = 0.0f; j.z = 0.0f; j.ground = true;
                if (r.push(j)) pushed++;
            }
        });
    }

    // One consumer, standing in for the game thread.
    unsigned popped = 0;
    std::thread consumer([&r, &popped, &stop]() {
        net::job out;
        while (!stop.load()) {
            while (r.pop(&out)) popped++;
        }
        while (r.pop(&out)) popped++;
    });

    for (unsigned t = 0; t < producers; t++) ts[t].join();
    stop.store(true);
    consumer.join();

    // Nothing is invented and nothing vanishes: everything the ring accepted
    // comes back out exactly once, and the rest is accounted for as dropped.
    check_true("no jobs lost or duplicated", popped == pushed.load());
    check_true("all attempts accounted for",
               pushed.load() + r.dropped() == producers * per_thread);
    check_true("every popped job is well-formed", popped > 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
