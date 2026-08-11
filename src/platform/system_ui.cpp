#include "system_ui.h"
#include "log.h"
#include "rage/invoker/invoker.h"

#include <orbis/libkernel.h>
#include <stdint.h>
#include <string.h>

namespace platform::system_ui {

    // Layout per the scene SDK system_service.h: the two flags we read sit at
    // fixed offsets 4 and 5, which every known header agrees on. reserved is
    // oversized well past the real struct (134 bytes) so a newer firmware
    // writing a longer status can never overrun our buffer.
    struct SceSystemServiceStatus {
        int32_t eventNum;
        bool    isSystemUiOverlaid;
        bool    isInBackgroundExecution;
        bool    isCpuMode7CpuNormal;
        bool    isGameLiveStreamingOnAir;
        bool    isOutOfVrPlayArea;
        uint8_t reserved[256];
    };

    typedef int32_t (*get_status_fn)(SceSystemServiceStatus*);

    static get_status_fn resolve_get_status() {
        OrbisKernelModule handles[256];
        size_t count = 0;
        if (sceKernelGetModuleList(handles, sizeof(handles) / sizeof(handles[0]), &count) != 0) {
            LOG_ERROR("system_ui: sceKernelGetModuleList failed - PS-button guard inactive");
            return nullptr;
        }

        for (size_t i = 0; i < count; ++i) {
            OrbisKernelModuleInfo info;
            memset(&info, 0, sizeof(info));
            info.size = sizeof(info);
            if (sceKernelGetModuleInfo(handles[i], &info) != 0)
                continue;
            if (!strstr(info.name, "libSceSystemService"))
                continue;

            void* addr = nullptr;
            if (sceKernelDlsym(handles[i], "sceSystemServiceGetStatus", &addr) == 0 && addr) {
                LOG("system_ui: sceSystemServiceGetStatus resolved from %s", info.name);
                return (get_status_fn)addr;
            }
        }

        LOG_ERROR("system_ui: sceSystemServiceGetStatus not found - PS-button guard inactive");
        return nullptr;
    }

    // The game keeps its own SceSystemServiceStatus in a global its event pump
    // (eboot 0x1954EE0) refreshes via sceSystemServiceGetStatus. Reading the
    // same bytes the game acts on needs no API resolution and cannot fail.
    // v1.57 RVAs: status @0x3AE64B4, isSystemUiOverlaid @+4, isInBackground-
    // Execution @+5.
    static constexpr uint64_t RVA_GAME_STATUS = 0x3AE64B4;

    bool overlaid() {
        // Primary: the game's cached flags.
        bool game_ui = false, game_bg = false;
        if (rage::invoker::g_eboot_base) {
            const uint8_t* st = (const uint8_t*)(rage::invoker::g_eboot_base + RVA_GAME_STATUS);
            game_ui = st[4] != 0;
            game_bg = st[5] != 0;
        }

        // Secondary: our own poll (fresher within a frame, and a safety net in
        // case the game-global RVA shifts on another eboot version).
        bool own_ui = false, own_bg = false;
        static bool resolved = false;
        static get_status_fn get_status = nullptr;
        if (!resolved) {
            resolved = true;
            get_status = resolve_get_status();
        }
        if (get_status) {
            SceSystemServiceStatus status;
            memset(&status, 0, sizeof(status));
            // Fail-open: an error here must not blank the menu forever.
            if (get_status(&status) == 0) {
                own_ui = status.isSystemUiOverlaid;
                own_bg = status.isInBackgroundExecution;
            }
        }

        bool now = game_ui || game_bg || own_ui || own_bg;

        // One-time klog so the nc capture shows whether detection is even live.
        static bool announced = false;
        if (!announced) {
            announced = true;
            platform::klogf("system_ui: guard active (dlsym=%d, game-status via base+0x%llx)",
                           get_status ? 1 : 0, (unsigned long long)RVA_GAME_STATUS);
        }

        static bool prev = false;
        if (now != prev) {
            prev = now;
            LOG("system_ui: overlay %s (game ui=%d bg=%d | own ui=%d bg=%d)",
                now ? "ENTER" : "LEAVE",
                (int)game_ui, (int)game_bg, (int)own_ui, (int)own_bg);
            platform::klogf("system_ui: overlay %s (game ui=%d bg=%d | own ui=%d bg=%d)",
                           now ? "ENTER" : "LEAVE",
                           (int)game_ui, (int)game_bg, (int)own_ui, (int)own_bg);
        }

        return now;
    }
}
