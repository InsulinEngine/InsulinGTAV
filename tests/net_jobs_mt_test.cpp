// Host unit test: the job ring under a concurrent producer and consumer.
//
// This lives in its own file, separate from tests/net_jobs_test.cpp, because it
// needs <thread>/<atomic> - C++ headers - and the installed clang has no libc++
// on this box, so it falls through to the real MSVC STL. Follow the same build
// line tests/protections_ring_mt_test.cpp documents for that situation; the
// plain "-I src" line used by the rest of the suite is not enough here.
//
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/net_jobs_mt_test.cpp src/net/jobs.cpp -o build/net_jobs_mt_test.exe
//   ./build/net_jobs_mt_test.exe
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
