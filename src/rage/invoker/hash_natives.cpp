#include "rage/invoker/hash_natives.h"
#include "rage/invoker/invoker.h"
#include "platform/log.h"

#include <string.h>

namespace rage::hash_natives {

    // The global rage::scrCommandHash that rage::scrThread::RegisterCommand feeds:
    // it tail-calls Insert(&g_scrCommandHashTable, hash, fn), so this is where all
    // ~6,700 registrations end up.
    static constexpr uint64_t RVA_COMMAND_HASH_TABLE = 0x3E0F910;

    // Node layout, derived from rage::scrCommandHash::Insert (0x25B0050). Every
    // field is XOR-masked with a per-node or per-entry key and, for the hash, with
    // the address of the field itself.
    //
    //   +0x00  next pointer low  ^ (key ^ (u32)node)
    //   +0x04  next pointer high ^ (key ^ (u32)node)
    //   +0x08  node key
    //   +0x10  function pointers, 8 bytes each
    //   +0x48  count ^ (u32)(node+0x48) ^ *(u32*)(node+0x4C)
    //   +0x4C  second half of the count encoding
    //   +0x54  per entry (stride 16): hash low  ^ entry key ^ (u32)&field
    //   +0x58  per entry (stride 16): hash high ^ entry key ^ (u32)&field(+0x54)
    //   +0x5C  per entry (stride 16): entry key
    //
    // A node holds at most 7 entries; Insert allocates a new one and links it at
    // the head of the bucket when the count reaches 7.
    static constexpr uint32_t BUCKET_COUNT   = 256;
    static constexpr uint32_t NODE_MAX_ENTRY = 7;
    static constexpr uint32_t OFF_NEXT_LO    = 0x00;
    static constexpr uint32_t OFF_NEXT_HI    = 0x04;
    static constexpr uint32_t OFF_NODE_KEY   = 0x08;
    static constexpr uint32_t OFF_FUNCS      = 0x10;
    static constexpr uint32_t OFF_COUNT_A    = 0x48;
    static constexpr uint32_t OFF_COUNT_B    = 0x4C;
    static constexpr uint32_t OFF_ENTRIES    = 0x54;
    static constexpr uint32_t ENTRY_STRIDE   = 0x10;

    // Longest bucket chain we will follow. Real chains are a handful of nodes;
    // anything longer means the decode went wrong and we are chasing garbage.
    static constexpr uint32_t MAX_CHAIN = 256;

    // eboot .text size from the RE catalog - used to sanity-check that a decoded
    // "function pointer" actually points into the game's code.
    static constexpr uint64_t EBOOT_TEXT_SIZE = 0x2ECC000;

    static constexpr uint32_t TABLE_SLOTS = 16384;   // power of two, load ~0.41
    static constexpr uint32_t TABLE_MASK  = TABLE_SLOTS - 1;

    struct entry {
        uint64_t hash;
        void*    fn;
    };

    static entry    g_table[TABLE_SLOTS];
    static uint32_t g_count = 0;
    static bool     g_usable = false;

    uint32_t entry_count() { return g_count; }
    bool usable() { return g_usable; }

    static void insert(uint64_t hash, void* fn) {
        if (!hash || g_count >= TABLE_SLOTS / 2)
            return;
        uint32_t i = (uint32_t)(hash ^ (hash >> 32)) & TABLE_MASK;
        for (;;) {
            if (g_table[i].hash == 0) {
                g_table[i].hash = hash;
                g_table[i].fn = fn;
                g_count++;
                return;
            }
            if (g_table[i].hash == hash) {
                g_table[i].fn = fn;
                return;
            }
            i = (i + 1) & TABLE_MASK;
        }
    }

    void* find(uint64_t hash) {
        if (!hash || !g_count)
            return nullptr;
        uint32_t i = (uint32_t)(hash ^ (hash >> 32)) & TABLE_MASK;
        for (uint32_t probes = 0; probes < TABLE_SLOTS; probes++) {
            if (g_table[i].hash == 0)
                return nullptr;
            if (g_table[i].hash == hash)
                return g_table[i].fn;
            i = (i + 1) & TABLE_MASK;
        }
        return nullptr;
    }

    static bool plausible_node(uintptr_t p) {
        // Heap pointers on this platform are far above the image and 16-byte
        // aligned. This will not catch every bad value, but it stops the obvious
        // ones before they are dereferenced.
        return p > 0x10000 && (p & 0xF) == 0;
    }

    bool build() {
        uintptr_t base = rage::invoker::g_eboot_base;
        if (!base)
            return false;

        memset(g_table, 0, sizeof(g_table));
        g_count = 0;
        g_usable = false;

        const uintptr_t code_lo = base;
        const uintptr_t code_hi = base + EBOOT_TEXT_SIZE;
        uintptr_t table = base + RVA_COMMAND_HASH_TABLE;

        uint32_t rejected = 0;

        for (uint32_t b = 0; b < BUCKET_COUNT; b++) {
            uintptr_t node = *(uintptr_t*)(table + 8ull * b);

            for (uint32_t chain = 0; node && chain < MAX_CHAIN; chain++) {
                if (!plausible_node(node))
                    break;

                uint32_t node_key = *(uint32_t*)(node + OFF_NODE_KEY);
                uint32_t ca = *(uint32_t*)(node + OFF_COUNT_A);
                uint32_t cb = *(uint32_t*)(node + OFF_COUNT_B);
                uint32_t count = ca ^ cb ^ (uint32_t)(node + OFF_COUNT_A);

                // The one field that tells us immediately whether the layout is
                // right: a node can never hold more than 7.
                if (count > NODE_MAX_ENTRY)
                    break;

                for (uint32_t i = 0; i < count; i++) {
                    uintptr_t field = node + OFF_ENTRIES + ENTRY_STRIDE * (uintptr_t)i;
                    uint32_t ekey = *(uint32_t*)(field + 8);
                    uint32_t lo = *(uint32_t*)(field)     ^ ekey ^ (uint32_t)field;
                    uint32_t hi = *(uint32_t*)(field + 4) ^ ekey ^ (uint32_t)field;
                    uint64_t hash = ((uint64_t)hi << 32) | lo;

                    void* fn = *(void**)(node + OFF_FUNCS + 8ull * i);
                    uintptr_t fp = (uintptr_t)fn;

                    // Every native handler lives in the eboot's code. Anything
                    // else means we decoded the wrong slot, and calling it later
                    // would be a jump into nowhere.
                    if (fp < code_lo || fp >= code_hi) {
                        rejected++;
                        continue;
                    }

                    insert(hash, fn);
                }

                uint32_t mask = node_key ^ (uint32_t)node;
                uint32_t nlo = *(uint32_t*)(node + OFF_NEXT_LO) ^ mask;
                uint32_t nhi = *(uint32_t*)(node + OFF_NEXT_HI) ^ mask;
                node = (uintptr_t)(((uint64_t)nhi << 32) | nlo);
            }
        }

        // klog, not the /data file: build() runs during the boot window, and
        // GoldHEN's FTP cannot hand the file back (RETR returns a fixed-size
        // blob whatever the file), so the only way to see these is live.
        platform::klogf("hash_natives: walked table: %u natives, %u rejected",
                        (unsigned)g_count, (unsigned)rejected);

        // Prove it before trusting it: resolve GET_HASH_KEY by hash, call it, and
        // compare against the joaat the RVA-based smoke test already checks.
        //
        // Use the CURRENT hash, not the b323 one. Published headers carry both -
        // the value passed to invoke<> is this build's, the one in the trailing
        // comment is the 2013 original. Reaching for the comment value is what
        // made the first attempt look like a build mismatch when the table was
        // fine all along.
        static constexpr uint64_t HASH_GET_HASH_KEY = 0x70E57E9927B6BA58ULL;
        static constexpr uint32_t EXPECTED = 0x0669D57Fu;

        void* fn = find(HASH_GET_HASH_KEY);
        if (!fn) {
            platform::klogf("hash_natives: GET_HASH_KEY not in the table - unusable");
            return false;
        }

        rage::invoker::native_setup ctx;
        ctx.push("insulin");
        ((rage::invoker::native_handler)fn)(&ctx);
        uint32_t got = ctx.get_return<uint32_t>();

        g_usable = (got == EXPECTED);
        platform::klogf("hash_natives: GET_HASH_KEY -> 0x%08X (%s)",
                        (unsigned)got, g_usable ? "OK, table usable" : "MISMATCH, table rejected");
        return g_usable;
    }
}
