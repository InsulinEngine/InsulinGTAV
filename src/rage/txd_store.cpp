#include "rage/txd_store.h"

namespace rage { namespace txd {
namespace {

    // Every read goes through volatile: the store is written by the streaming
    // system on other threads, and a re-read of a field the compiler thinks is
    // loop-invariant is exactly what would turn a torn update into a wild
    // pointer.
    uint64_t rd64(uint64_t a) { return *(const volatile uint64_t*)a; }
    uint32_t rd32(uint64_t a) { return *(const volatile uint32_t*)a; }
    uint8_t  rd8 (uint64_t a) { return *(const volatile uint8_t*)a;  }

    // The game trusts its own table and walks it unbounded. We cannot: if a
    // future build moves these fields, the "nodes" pointer is whatever happens
    // to sit at +0x70, and an unbounded walk over it would hang or fault the
    // game thread instead of failing the lookup. Both caps are sanity limits,
    // not the table's real size - the store does not publish a node count.
    const uint32_t max_node_index = 1u << 20;   // 12 MB of nodes; a real store is far smaller
    const int      max_walk       = 4096;       // chain length before giving up
}

int slot_from_hash(uint64_t store, uint32_t name_hash) {
    const uint64_t nodes        = rd64(store + 0x70);
    const uint64_t buckets      = rd64(store + 0x78);
    const uint32_t bucket_count = rd32(store + 0x80);
    if (!nodes || !buckets || !bucket_count) return no_slot;

    uint32_t i = rd32(buckets + 4ULL * (name_hash % bucket_count));
    for (int walked = 0; walked < max_walk; walked++) {
        if (i == no_index || i >= max_node_index) return no_slot;
        const uint64_t node = nodes + 12ULL * i;
        if (rd32(node) == name_hash) {
            const int32_t slot = (int32_t)rd32(node + 4);
            return slot < 0 ? no_slot : (int)slot;   // the node itself can say "none"
        }
        i = rd32(node + 8);
    }
    return no_slot;                                  // cycle or corrupt chain
}

uint64_t dict_from_slot(uint64_t store, int slot) {
    if (slot < 0) return 0;

    // Order matters: the flags byte is what says whether the pool entry is
    // meaningful at all, so it is checked before the entry is read.
    const uint64_t flags = rd64(store + 0x40);
    if (!flags) return 0;
    if (rd8(flags + (uint32_t)slot) & 0x80) return 0;

    const uint64_t pool   = rd64(store + 0x38);
    const uint32_t stride = rd32(store + 0x4C);
    if (!pool || !stride) return 0;
    return rd64(pool + (uint64_t)stride * (uint32_t)slot);
}

}}
