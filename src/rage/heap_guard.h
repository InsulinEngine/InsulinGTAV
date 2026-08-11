#pragma once

// Guards the RAGE general-allocator's page-index resolver against foreign
// pointers -- the fix for the PS-button texture crash.
//
// When GTA suspends (PS button -> system overlay) it runs a resource-accounting
// pass that asks the allocator to translate every live block pointer into a
// heap page/chunk index (roughly ptr - heap_base). A pointer that was NOT
// allocated from the RAGE heap -- e.g. a libc-malloc'd block injected into the
// txd store -- lies below heap_base, so the translation yields a wild index ->
// out-of-bounds read -> SIGSEGV.
//
// This detour clamps that case to a safe page-aligned value instead of
// faulting. It is the global safety net: rage::gfx already allocates injected
// blocks from the RAGE heap so they resolve correctly (see rage/gfx.cpp), but
// this hook also covers the malloc fallback path that gfx.cpp otherwise flags
// as "suspend may crash", and any other foreign pointer the pass encounters.

namespace rage::heap_guard {

    // Detour target is sub_195F870 in the v1.57 eboot (RVA, imagebase 0); see
    // heap_guard.cpp for the derivation and disassembly. Complements the
    // rage::gfx mitigations (RAGE-heap allocation + vtable adoption) as a global
    // net that also covers gfx.cpp's malloc fallback path and any other foreign
    // pointer the accounting pass encounters.
    //
    // Install the page-resolver detour. Requires rage::invoker::g_eboot_base to
    // be resolved first. Returns false if the hook could not be installed.
    // Idempotent: a second call while already installed is a no-op that returns
    // true.
    bool install();
}
