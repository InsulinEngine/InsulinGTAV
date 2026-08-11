#include "rage/heap_guard.h"
#include "rage/invoker/invoker.h"
#include "platform/log.h"

#include <GoldHEN/Detour.h>
#include <stdint.h>

// Fix for the PS-button suspend crash, ported from a fix the original author
// wrote against a v1.57 eboot that they had REBASED to 0x400000. Two adjustments
// for this build:
//   * their address 0x1D5F870 is 0x400000-relative; this project uses imagebase
//     0, so the real RVA is 0x1D5F870 - 0x400000 = 0x195F870, made absolute at
//     install time via g_eboot_base (the loaded eboot is relocated).
//   * the original is chained through Detour_Stub (this SDK has no HOOK_CONTINUE).
//
// Verified against the v1.57 IDB -- sub_195F870(self=rdi, ptr=rsi):
//     mov rdx,[rdi+0BD8h]   ; heap base
//     mov r8, [rdi+0BE0h]   ; 64 KiB page count
//     sub rsi,rdx / shr rax,10h / cmp rax,r8   ; page index = (ptr-base)>>16
//     jnb ... [rdi+5E8h + (idx-count)*8]       ; big-block table
// For ptr<base the subtraction underflows -> huge index -> wild read -> the
// crash. The guard's constants (0xBD8, the 64 KiB mask) match the disassembly;
// it only diverges from the original when ptr<base, which is never true for a
// real block of this heap, so legitimate lookups are unaffected.
namespace rage::heap_guard {

    // Allocator ptr->page resolver. RVA into the CUSA00411 v1.57 eboot
    // (imagebase 0) = author's 0x1D5F870 minus their 0x400000 rebase.
    static constexpr uint64_t RVA_PAGE_RESOLVER      = 0x195F870;

    // The allocator's heap-base field (verified: `mov rdx,[rdi+0BD8h]`). A query
    // pointer below this is not one of the allocator's own blocks.
    static constexpr uint64_t ALLOCATOR_HEAPBASE_OFF = 0xBD8;

    // Align-down mask: clears the low 16 bits (64 KiB page). A foreign pointer
    // is snapped to its page instead of being turned into a wild heap index.
    static constexpr uint64_t PAGE_ALIGN_MASK        = 0xFFFFFFFFFFFF0000ULL;

    typedef uint64_t (*page_resolver_fn)(void* self, uint64_t ptr);

    static Detour g_detour;
    static bool   g_installed = false;

    static uint64_t page_resolver_hook(void* self, uint64_t ptr) {
        if (self == nullptr || ptr == 0)
            return 0;

        uint64_t heap_base = *(uint64_t*)((uint8_t*)self + ALLOCATOR_HEAPBASE_OFF);
        if (heap_base != 0 && ptr < heap_base)
            return ptr & PAGE_ALIGN_MASK;   // foreign pointer: clamp, don't index

        return Detour_Stub(&g_detour, page_resolver_fn, self, ptr);
    }

    bool install() {
        if (g_installed)
            return true;
        if (!rage::invoker::g_eboot_base)
            return false;

        Detour_Construct(&g_detour, DetourMode_x64);
        void* stub = Detour_DetourFunction(
            &g_detour,
            rage::invoker::g_eboot_base + RVA_PAGE_RESOLVER,
            (void*)&page_resolver_hook);

        g_installed = (stub != nullptr);
        if (g_installed)
            platform::logf("heap_guard", "page-resolver guard installed @ base+0x%llx",
                           (unsigned long long)RVA_PAGE_RESOLVER);
        else
            LOG_ERROR("heap_guard: failed to install page-resolver detour");
        return g_installed;
    }
}
