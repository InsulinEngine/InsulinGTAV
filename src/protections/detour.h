#pragma once
#include <stdint.h>
#include <GoldHEN/Detour.h>

// Thin wrapper over the GoldHEN Detour API, matching the pattern proven in
// rage/heap_guard.cpp. Each filter owns one slot; a slot is installed once and
// never removed.
namespace protections {

    // The flags carry default member initialisers even though every instance is
    // a namespace-scope static (zero-initialised, so correct today). This makes
    // it correct by construction rather than by where it happens to be
    // declared - a stack or heap detour_slot would otherwise start with garbage
    // flags and install_detour() would skip the construct or the install.
    //
    // `d` MUST carry `{}` alongside them, and that is not cosmetic. The
    // implicitly-defined default constructor is only constexpr if EVERY
    // non-static member is initialised; leaving `Detour d;` bare while the two
    // bools have initialisers makes it non-constexpr, which demotes each
    // namespace-scope slot from constant initialisation to DYNAMIC
    // initialisation - an .init_array entry per translation unit. This plugin
    // has no .init_array: those entries never run. Measured, not reasoned
    // about: `Detour d;` + `= false` grew .init_array from 0x320 to 0x348 (one
    // dead entry per hooks_*.cpp). With `Detour d{}` it is back to 0x320 and
    // every slot is constant-initialised straight into .bss.
    struct detour_slot {
        Detour d{};
        bool   constructed = false;
        bool   installed   = false;
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
