#pragma once
#include <stdint.h>
#include <GoldHEN/Detour.h>

// Thin wrapper over the GoldHEN Detour API, matching the pattern proven in
// rage/heap_guard.cpp. Each filter owns one slot; a slot is installed once and
// never removed.
namespace protections {

    struct detour_slot {
        Detour d;
        bool   constructed;
        bool   installed;
    };

    // Resolves `rva` against g_eboot_base and detours it to `hook`.
    // Idempotent: a second call on an installed slot returns true and does
    // nothing. Returns false if the base is unresolved or the detour failed.
    bool install_detour(detour_slot* slot, uint64_t rva, void* hook);
}

// Call the original from inside a hook. Pass a TYPEDEF'd function-pointer type,
// never an inline one: an inline `void(*)(void*, int)` contains commas, which
// the preprocessor splits into separate macro arguments.
//   typedef void (*my_fn)(void*, int);
//   PROT_CHAIN(g_slot, my_fn, a, b)
#define PROT_CHAIN(slot, ...) Detour_Stub(&(slot).d, __VA_ARGS__)
