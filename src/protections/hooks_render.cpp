#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "rage/invoker/invoker.h"

// Draw-list overflow guards. A remote player can push the render entity list
// past its capacity; the game does not bound-check the add, and the overflow
// corrupts whatever follows the array. Each guard refuses the add that would
// cross the line, which costs one missing entity for one frame.
//
// The count offset and capacity below are read out of the PS4 disassembly, not
// carried over from the PC build - see analysis/PROTECTIONS_ANCHORS.md in the
// IDA repo (E:\Projects\IDA\PS4\GTA5). The short version:
//
//   sub_1E5AF50 @ 0x1E5AF50 is the only place a command slot is handed out:
//       mov  eax, [rdi+14720h]     ; count
//       cmp  eax, 200h             ; the game's OWN capacity check = 512
//       jnz  ok
//       int  41h                   ; trap - there is no graceful path past 512
//   ok: lea  ecx, [rax+1]
//       mov  [rdi+14720h], ecx
//       lea  rax, [rax+rax*4] / shl rax,5 / lea rax,[rdi+rax+720h]
//   0x720 + 512*160 = 0x14720, so the counter sits immediately after the array
//   and the 513th add overwrites the counter itself.
//
// PC's offset is 0x14730, not 0x14720: the PS4 manager object is 16 bytes
// smaller (0x149D0 vs 0x149E0) and every field from the array on is shifted
// down by 16. Using YimMenu's PC constant here would read the object's tail
// instead of the counter.
namespace protections {
namespace {
    // eboot RVAs, CUSA00411 v1.57, imagebase 0.
    const uint64_t RVA_RENDER_PED       = 0x7FCA00;  // sub_7FCA00, PC sub_7FF6FE5F7B5C
    const uint64_t RVA_RENDER_ENTITY    = 0x7E0CA0;  // sub_7E0CA0, PC sub_7FF6FE5EA174
    const uint64_t RVA_RENDER_BIG_PED   = 0x7FC4A0;  // sub_7FC4A0, PC sub_7FF6FE5F34F4
    const uint64_t RVA_DRAW_HANDLER_MGR = 0x3E1D1C0; // qword holding the manager pointer
    const uint64_t COUNT_OFFSET         = 0x14720;   // PC 0x14730 - do not port that one
    const int      CAPACITY             = 512;       // from `cmp eax, 200h` in sub_1E5AF50
    const int      PED_HEADROOM         = 13;        // YimMenu bails at 499 of 512

    typedef void* (*render_ped_fn)(void*, void*, void*, void*);
    typedef void  (*render_entity_fn)(void*, void*, int, bool);
    typedef void* (*render_big_ped_fn)(void*, void*, void*, void*);

    detour_slot g_ped;
    detour_slot g_entity;
    detour_slot g_big_ped;

    int draw_list_count() {
        if (!rage::invoker::g_eboot_base) return 0;
        void** mgr_ptr = (void**)(rage::invoker::g_eboot_base + RVA_DRAW_HANDLER_MGR);
        void*  mgr     = *mgr_ptr;
        if (!mgr) return 0;
        return *(int*)((uint8_t*)mgr + COUNT_OFFSET);
    }

    void* render_ped_hook(void* renderer, void* ped, void* a3, void* a4) {
        if (should_report(filter_id::render_ped)) {
            const int count = draw_list_count();
            if (count >= CAPACITY - PED_HEADROOM) {
                report(filter_id::render_ped, -1, 0, (uint32_t)count, 0);
                if (should_block(filter_id::render_ped))
                    return nullptr;
            }
        }
        return PROT_CHAIN(g_ped, render_ped_fn, renderer, ped, a3, a4);
    }

    void render_entity_hook(void* renderer, void* entity, int unk, bool a4) {
        if (should_report(filter_id::render_entity)) {
            const int count = draw_list_count();
            if (count >= CAPACITY) {
                report(filter_id::render_entity, -1, 0, (uint32_t)count, 0);
                if (should_block(filter_id::render_entity)) {
                    // The sentinel the caller expects for "no entry produced".
                    int* flags = (int*)((uint8_t*)renderer + 4);
                    *flags &= ~0x80000000;
                    *flags &= ~0x40000000;
                    *flags |= (a4 & 1) << 30;
                    *(int*)renderer = -2;
                    return;
                }
            }
        }
        PROT_CHAIN(g_entity, render_entity_fn, renderer, entity, unk, a4);
    }

    void* render_big_ped_hook(void* renderer, void* ped, void* a3, void* a4) {
        if (should_report(filter_id::render_big_ped)) {
            const int count = draw_list_count();
            if (count >= CAPACITY) {
                report(filter_id::render_big_ped, -1, 0, (uint32_t)count, 0);
                if (should_block(filter_id::render_big_ped)) {
                    *(int*)((uint8_t*)a4 + 4) = -2;
                    return (uint8_t*)a4 + 0x14;
                }
            }
        }
        return PROT_CHAIN(g_big_ped, render_big_ped_fn, renderer, ped, a3, a4);
    }
}

bool install_render_ped()     { return install_detour(&g_ped,     RVA_RENDER_PED,     (void*)&render_ped_hook); }
bool install_render_entity()  { return install_detour(&g_entity,  RVA_RENDER_ENTITY,  (void*)&render_entity_hook); }
bool install_render_big_ped() { return install_detour(&g_big_ped, RVA_RENDER_BIG_PED, (void*)&render_big_ped_hook); }
}
