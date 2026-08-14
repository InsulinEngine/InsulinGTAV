#pragma once
#include <stdint.h>
#include "rage/invoker/invoker.h"

// Call natives by their 64-bit hash instead of by RVA.
//
// The generated natives.h covers 2,840 natives; the build registers ~6,700. The
// rest exist in the eboot and are simply absent from that header, which is why
// features keep hitting "this native does not exist" when it does.
//
// Detouring the registration does not work here: GoldHEN loads the plugin after
// the game has already registered everything, so the hook never fires (measured -
// the table came back empty). What does work is reading the finished table, which
// is still sitting in memory: g_scrCommandHashTable, 256 buckets of chained nodes.
//
// The table is XOR-obfuscated, so every field is decoded rather than read, and
// every decoded value is range-checked before use - a wrong guess about the
// layout would otherwise hand out garbage function pointers. build() reports
// what it recovered and verifies the result against a known native before
// declaring the table usable.
namespace rage::hash_natives {

    // Walks g_scrCommandHashTable and fills the lookup table. Safe to call once
    // the eboot base is resolved; the game has long since registered by then.
    // Returns true only if the result also passed verification.
    bool build();

    // How many hash->function pairs were recovered.
    uint32_t entry_count();

    // True once build() succeeded and verified. While false, invoke_hash refuses
    // to call anything.
    bool usable();

    // Function registered for this hash, or nullptr.
    void* find(uint64_t hash);

    // Call a native by hash. Same shape as rage::invoker::invoke, so a hash-based
    // natives.h can be pointed at this. Returns a default-constructed R when the
    // hash is not registered in this build - the normal outcome for a header
    // written against a different build, so check find(hash) when it matters.
    template<typename R, typename... Args>
    R invoke_hash(uint64_t hash, Args&&... args) {
        if (!usable())
            return R();
        void* fn = find(hash);
        if (!fn)
            return R();
        rage::invoker::native_setup ctx;
        rage::invoker::pass{ ([&]() { ctx.push(args); }(), 1)... };
        ((rage::invoker::native_handler)fn)(&ctx);
        return ctx.get_return<R>();
    }
}
