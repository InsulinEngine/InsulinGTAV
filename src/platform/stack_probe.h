#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <orbis/libkernel.h>
#include "platform/log.h"

// Boot-time stack accounting. Diagnostics only - remove once the boot crash is
// closed out.
//
// Why this exists: menu::build() runs from module_start, so it executes on
// whatever thread GoldHEN happens to load the plugin on, and the submenu load()
// functions have very large frames. Each add_option() line leaves an option
// temporary (~1.3 KB: two localization members plus several stl::function) live
// to the end of the function and the compiler does not reuse the slots, so the
// frame scales with the option count - measured in the built .elf:
//
//   main_menu::load()      49,968 bytes
//   vehicle_menu::load()   43,744 bytes
//   player_menu::load()    36,480 bytes
//
// Nothing tells us how much stack that thread has. And clang emits no stack
// probes on this target, so a frame larger than the remaining stack does not
// fault on the guard page - it jumps clean over it and writes into whatever is
// mapped below. That corrupts a neighbouring mapping and takes the game down
// later, at a moment that depends on how the process was laid out on that boot:
// exactly the "crashes on the loading screen, boots fine the next time"
// signature we are chasing.
//
// So measure it instead of guessing. Two independent sources, because they can
// disagree and the disagreement is itself informative:
//
//   sceKernelVirtualQuery - the mapping the stack pointer actually lives in.
//                           Works regardless of which thread we are on or who
//                           created it.
//   scePthreadAttrGetstack - what the thread's own bookkeeping claims.
namespace platform {

    inline void stack_report(const char* where) {
        uintptr_t sp = (uintptr_t)__builtin_frame_address(0);

        OrbisKernelVirtualQueryInfo vq;
        memset(&vq, 0, sizeof(vq));
        int vq_ret = sceKernelVirtualQuery((const void*)sp, 0, &vq, sizeof(vq));
        if (vq_ret == 0) {
            uintptr_t lo = (uintptr_t)vq.unk01;
            uintptr_t hi = (uintptr_t)vq.unk02;
            // headroom = what is left below us before the mapping ends. This is
            // the number that has to cover the deepest load() frame.
            klogf("stack[%s] sp=0x%llx map=0x%llx-0x%llx size=%llu headroom=%llu isStack=%d name=%.32s",
                  where,
                  (unsigned long long)sp,
                  (unsigned long long)lo,
                  (unsigned long long)hi,
                  (unsigned long long)(hi > lo ? hi - lo : 0),
                  (unsigned long long)(sp > lo ? sp - lo : 0),
                  (int)vq.isStack,
                  vq.name);
        } else {
            klogf("stack[%s] sp=0x%llx virtualquery failed ret=0x%x",
                  where, (unsigned long long)sp, (unsigned)vq_ret);
        }

        OrbisPthreadAttr attr;
        if (scePthreadAttrInit(&attr) == 0) {
            if (scePthreadAttrGet(scePthreadSelf(), &attr) == 0) {
                void*  base = NULL;
                size_t size = 0;
                if (scePthreadAttrGetstack(&attr, &base, &size) == 0) {
                    uintptr_t b = (uintptr_t)base;
                    klogf("stack[%s] attr base=0x%llx size=%llu used=%lld",
                          where,
                          (unsigned long long)b,
                          (unsigned long long)size,
                          (long long)((b + size) - sp));
                } else {
                    klogf("stack[%s] attr getstack failed", where);
                }
            } else {
                klogf("stack[%s] attr get failed", where);
            }
            scePthreadAttrDestroy(&attr);
        } else {
            klogf("stack[%s] attr init failed", where);
        }
    }
}
