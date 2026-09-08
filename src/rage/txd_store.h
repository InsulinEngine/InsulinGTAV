#pragma once

#include <stdint.h>

// fwTxdStore lookups, expressed as pure reads over the store's own tables so
// they can be host-tested (tests/txd_store_test.cpp) and so a per-frame scan
// never has to call into game code.
//
// Name-hash -> slot is an open hash map living beside the pool.
// CLoadingScreens::SetupLogoSprite (eboot RVA 0x6EAF0) walks it inline:
//     buckets = *(u32**)(store + 0x78);  bucket_count = *(u32*)(store + 0x80);
//     nodes   = *(node**)(store + 0x70);        // { u32 hash, s32 slot, u32 next }
//     i = buckets[hash % bucket_count];
//     while (nodes[i].hash != hash) { i = nodes[i].next; if (i == ~0u) miss; }
//     slot = nodes[i].slot;                     // -1 means "no slot"
//
// Slot -> pgDictionary* uses the pool side (+0x38 base, +0x40 per-slot flags,
// +0x4C stride), which g_get_sprite_texture (0x9BBB30) confirms independently,
// including that an entry whose flags carry 0x80 is masked to 0.
//
// NOT the vtable. vtable+0x48 takes a `const char*`, not a hash - it runs
// atStringHash on its argument. Passing it a hash dereferences the hash as a
// pointer, which is a SIGSEGV in atStringHash+0xB (`mov al, [rdi]`). That is
// measured, not theorised: the vehicle preview did exactly this and the fault
// address was 0xB63C4BA0 == atStringHash("candc_apartments"), the first dict
// it looked up. If a name-keyed lookup is ever wanted, pass the name.
namespace rage { namespace txd {

    const int      no_slot  = -1;
    const uint32_t no_index = 0xFFFFFFFFu;

    // Dict-name hash -> store slot, or no_slot if the store does not have it.
    int slot_from_hash(uint64_t store, uint32_t name_hash);

    // Store slot -> pgDictionary*, or 0 when the slot is empty or flagged
    // invalid. A dictionary that has not finished streaming reads as 0 here,
    // which is why callers retry rather than treat it as absent.
    uint64_t dict_from_slot(uint64_t store, int slot);
}}
