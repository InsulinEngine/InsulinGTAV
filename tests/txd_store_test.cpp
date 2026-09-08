// Host unit tests for the fwTxdStore name-hash -> slot lookup.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/txd_store_test.cpp src/rage/txd_store.cpp -o build/txd_store_test.exe
//   ./build/txd_store_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
//
// The store is faked in host memory with the layout the eboot uses, so every
// branch of the walk is exercised without the console: hit at the bucket head,
// hit after a chain walk, both flavours of miss, the flags&0x80 invalid slot,
// and the malformed-table cases that must return rather than fault or hang.
#include "rage/txd_store.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void check_eq(const char* what, long long got, long long want) {
    if (got != want) { printf("FAIL %s: got %lld, want %lld\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %lld\n", what, got); }
}

// ---- fake store -----------------------------------------------------------
// Only the fields the lookup reads are populated; everything else stays zero,
// which is also how a half-initialised store would look at boot.
namespace {

    struct node { uint32_t hash; int32_t slot; uint32_t next; };

    unsigned char g_store[0x100];
    node          g_nodes[8];
    uint32_t      g_buckets[4];
    uint64_t      g_pool[8];
    unsigned char g_flags[8];

    void put64(unsigned off, uint64_t v) { memcpy(g_store + off, &v, 8); }
    void put32(unsigned off, uint32_t v) { memcpy(g_store + off, &v, 4); }

    uint64_t store_addr() { return (uint64_t)(uintptr_t)g_store; }

    void build_store() {
        memset(g_store, 0, sizeof(g_store));
        memset(g_nodes, 0, sizeof(g_nodes));
        memset(g_pool,  0, sizeof(g_pool));
        memset(g_flags, 0, sizeof(g_flags));
        for (unsigned i = 0; i < 4; i++) g_buckets[i] = rage::txd::no_index;

        put64(0x38, (uint64_t)(uintptr_t)g_pool);     // pool base
        put64(0x40, (uint64_t)(uintptr_t)g_flags);    // per-slot flags
        put32(0x4C, 8);                               // stride: one qword per slot
        put64(0x70, (uint64_t)(uintptr_t)g_nodes);    // node array
        put64(0x78, (uint64_t)(uintptr_t)g_buckets);  // bucket heads
        put32(0x80, 4);                               // bucket count
    }
}

int main() {
    using namespace rage::txd;

    // ---- hit at the bucket head -------------------------------------------
    build_store();
    // 0xB63C4BA0 is atStringHash("candc_apartments") - the exact value that
    // took the console down when it was passed to vtable+0x48 as a pointer.
    const uint32_t h_apartments = 0xB63C4BA0u;
    g_nodes[0].hash = h_apartments; g_nodes[0].slot = 41; g_nodes[0].next = no_index;
    g_buckets[h_apartments % 4] = 0;
    check_eq("head hit -> slot", slot_from_hash(store_addr(), h_apartments), 41);

    // ---- hit after walking the chain --------------------------------------
    build_store();
    const uint32_t h_a = 0x10000004u, h_b = 0x20000004u, h_c = 0x30000004u;  // all % 4 == 0
    g_nodes[0].hash = h_a; g_nodes[0].slot = 1; g_nodes[0].next = 1;
    g_nodes[1].hash = h_b; g_nodes[1].slot = 2; g_nodes[1].next = 2;
    g_nodes[2].hash = h_c; g_nodes[2].slot = 3; g_nodes[2].next = no_index;
    g_buckets[0] = 0;
    check_eq("chain hit first",  slot_from_hash(store_addr(), h_a), 1);
    check_eq("chain hit middle", slot_from_hash(store_addr(), h_b), 2);
    check_eq("chain hit last",   slot_from_hash(store_addr(), h_c), 3);

    // ---- misses ------------------------------------------------------------
    check_eq("miss: chain ends",  slot_from_hash(store_addr(), 0x40000004u), no_slot);
    check_eq("miss: empty bucket", slot_from_hash(store_addr(), 0x40000001u), no_slot);

    // A node may itself say "no slot"; that is a miss, not slot -1.
    build_store();
    g_nodes[0].hash = h_a; g_nodes[0].slot = -1; g_nodes[0].next = no_index;
    g_buckets[0] = 0;
    check_eq("node holds no slot", slot_from_hash(store_addr(), h_a), no_slot);

    // ---- malformed tables must return, never fault or hang -----------------
    build_store();
    put64(0x70, 0);
    check_eq("null node array", slot_from_hash(store_addr(), h_a), no_slot);
    build_store();
    put64(0x78, 0);
    check_eq("null buckets", slot_from_hash(store_addr(), h_a), no_slot);
    build_store();
    put32(0x80, 0);
    check_eq("zero buckets (no modulo by zero)", slot_from_hash(store_addr(), h_a), no_slot);

    // A cycle is the one shape that could hang the game thread. The walk is
    // capped, so this must come back rather than spin.
    build_store();
    g_nodes[0].hash = h_a + 1; g_nodes[0].slot = 1; g_nodes[0].next = 1;
    g_nodes[1].hash = h_a + 2; g_nodes[1].slot = 2; g_nodes[1].next = 0;
    g_buckets[0] = 0;
    check_eq("cycle terminates", slot_from_hash(store_addr(), h_a), no_slot);

    // An index past the node array would read out of bounds; the store's own
    // count is not published, so the cap doubles as the bound.
    build_store();
    g_buckets[0] = 0x00FFFFFEu;
    check_eq("absurd head index", slot_from_hash(store_addr(), h_a), no_slot);

    // ---- slot -> pgDictionary ---------------------------------------------
    build_store();
    g_pool[3] = 0xDEADBEEF00ULL;
    check_eq("dict from slot", (long long)dict_from_slot(store_addr(), 3), (long long)0xDEADBEEF00ULL);

    g_flags[3] = 0x80;
    check_eq("flagged slot is invalid", (long long)dict_from_slot(store_addr(), 3), 0);
    g_flags[3] = 0x7F;                       // every other bit is not our business
    check_eq("other flag bits ignored", (long long)dict_from_slot(store_addr(), 3),
             (long long)0xDEADBEEF00ULL);

    check_eq("negative slot", (long long)dict_from_slot(store_addr(), no_slot), 0);

    build_store();
    put64(0x38, 0);
    check_eq("null pool", (long long)dict_from_slot(store_addr(), 3), 0);
    build_store();
    put64(0x40, 0);
    check_eq("null flags", (long long)dict_from_slot(store_addr(), 3), 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
