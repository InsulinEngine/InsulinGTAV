#include "protections/detour.h"
#include "rage/invoker/invoker.h"
#include "platform/log.h"

namespace protections {

bool install_detour(detour_slot* slot, uint64_t rva, void* hook)
{
    if (!slot || !hook) return false;
    if (slot->installed) return true;
    if (!rage::invoker::g_eboot_base) {
        LOG_ERROR("protections: detour @rva 0x%llx skipped - base unresolved",
                  (unsigned long long)rva);
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
