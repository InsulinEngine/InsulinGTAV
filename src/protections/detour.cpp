#include "protections/detour.h"
#include "protections/branch_check.h"
#include "rage/invoker/invoker.h"
#include "platform/log.h"

namespace protections {

bool install_detour(detour_slot* slot, uint64_t rva, void* hook)
{
    if (!slot || !hook) return false;
    // A zero rva is an unfilled TODO-ANCHOR, not an address. Without this the
    // base + 0 that follows is non-null, so Detour_DetourFunction would go on to
    // disassemble and patch the loaded ELF header; whether that fails safely
    // depends on HDE64 happening to raise F_ERROR on the ELF magic bytes,
    // which is not a guarantee. Refuse it loudly - a filter that never
    // installs should say so.
    if (!rva) {
        LOG_ERROR("protections: detour skipped - rva is zero (unfilled anchor)");
        return false;
    }
    if (slot->installed) return true;
    if (!rage::invoker::g_eboot_base) {
        LOG_ERROR("protections: detour @rva 0x%llx skipped - base unresolved",
                  (unsigned long long)rva);
        return false;
    }

    // Read the target's first 32 bytes and refuse a prologue the SDK's
    // non-relocating memcpy would corrupt. A refusal here is a filter that
    // does not install - loud, and in the log. The alternative is a stub that
    // jumps to a wrong address on a path we cannot debug from here.
    const uint8_t* target = (const uint8_t*)(rage::invoker::g_eboot_base + rva);
    const prologue_verdict v = check_prologue(target, 32);
    if (!v.safe) {
        LOG_ERROR("protections: detour @rva 0x%llx REFUSED - %s",
                  (unsigned long long)rva, v.reason ? v.reason : "unsafe prologue");
        return false;
    }

    if (!slot->constructed) {
        Detour_Construct(&slot->d, DetourMode_x64);
        slot->constructed = true;
    }

    void* stub = Detour_DetourFunction(&slot->d,
                                       rage::invoker::g_eboot_base + rva,
                                       hook);
    slot->installed = (stub != nullptr);

    if (slot->installed)
        platform::klogf("prot detour installed @ base+0x%llx", (unsigned long long)rva);
    else
        LOG_ERROR("protections: detour @rva 0x%llx FAILED", (unsigned long long)rva);

    return slot->installed;
}
}
