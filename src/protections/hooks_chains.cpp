#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "rage/invoker/invoker.h"

// Pointer-chain guards. Two crash sites that share one shape: the original
// walks a pointer chain a crafted sync can break, and the guard declines the
// call when the chain does not hold.
//
// Evidence lives in analysis/PROTECTIONS_ANCHORS.md in the IDA repo
// (E:\Projects\IDA\PS4\GTA5), section 8. Both targets were located from
// YimMenu's PC byte signatures resolved against the PC dump, and every constant
// below was then re-read out of the PS4 disassembly.
//
// The third target of this group, fragment_physics_crash_2(float*, float*), was
// NOT identified on PS4 and is deliberately not hooked; its registry entry keeps
// a null installer. See the report for what was tried.
namespace protections {
namespace {
    // eboot RVAs, CUSA00411 v1.57, imagebase 0.
    const uint64_t RVA_INVALID_DECAL = 0x6E13F0;    // CPedVariationStream::SetPaletteTexture
    const uint64_t RVA_SEARCHLIGHT   = 0x108F770;   // searchlight sweep-target position

    // CSearchLight's vtable, used as a pure-read replacement for the game's
    // "does this ped have a searchlight" accessor. Confirmed two ways: slot 7 is
    // CSearchLight::ProcessPostPreRender (sub_136CC20, the only function that
    // reads the three searchlight VisualSettings blocks), and slot 11 -
    // GetType() - is sub_23CF690, whose entire body is "return 1", which is
    // VGT_SEARCHLIGHT. Slot 11 is exactly the slot the original calls and
    // compares against 1.
    const uint64_t RVA_VT_CSEARCHLIGHT = 0x306FD10;

    // Read off the PS4 target, not ported. CPed +0x14B0 is m_pMyVehicle
    // (PC +5424; the -128 shift matches the CPed shifts already recorded in
    // sections 5 and 7). CVehicle +0xC50 is m_pVehicleWeaponMgr (PC +3200).
    const uint64_t PED_MY_VEHICLE   = 0x14B0;   // 5296
    const uint64_t VEH_WEAPON_MGR   = 0x0C50;   // 3152
    const uint64_t MGR_WEAPON_ARRAY = 0x0068;   // 104
    const uint64_t MGR_WEAPON_COUNT = 0x0114;   // 276

    // CVehicleWeaponMgr: m_pTurrets[12] @ +8, m_pVehicleWeapons[6] @ +104,
    // m_WeaponSeatIndices[6], m_TurretSeatIndices[12], m_TurretWeaponIndices[12],
    // m_iNumTurrets @ +272, m_iNumVehicleWeapons @ +276. Solving
    // 104 + 8W + 4W + 48 + 48 = 272 gives W = 6, which is the array bound the
    // loop below must not walk past. A count outside [1, 6] is itself proof the
    // object is not a live weapon manager.
    const int MGR_MAX_WEAPONS = 6;

    // The decal chain. All three offsets held from PC to PS4 and were re-read
    // out of sub_6E13F0 itself.
    const uint64_t PED_DRAW_HANDLER = 0x48;
    const uint64_t DH_VAR_DATA      = 0x30;
    const uint64_t VAR_PALETTE_SET  = 0x2C8;   // 712

    typedef void (*invalid_decal_fn)(uint64_t, int);
    typedef void (*searchlight_fn)(void*, void*);

    detour_slot g_decal;
    detour_slot g_searchlight;

    // CPedVariationStream::SetPaletteTexture(CPed* ped, u32 component)
    // @ 0x6E13F0, PC twin 0x7FF6FE78BC38 (RVA 0x6ABC38).
    //
    //   v40 = *(qword*)(*(qword*)(ped + 0x48) + 0x30);   // <- link 1 UNCHECKED
    //   if (v40) {
    //     if (component == 5 || GetCrewEmblem(ped) != 1) {
    //        ... *(qword*)(*(qword*)(v40 + 0x2C8) + 8*component + 184) = tex;
    //                     ^^^^^^^^^^^^^^^^^^^^^^^ link 3, UNCHECKED here
    //     } else {
    //        v29 = *(qword*)(v40 + 0x2C8);
    //        if (v29) { if (component == 2) { *(qword*)(v29 + 200) = ...; } ... }
    //                   ^^^^^^^^^^^^^^^^^^ link 3, checked on this branch only
    //     }
    //   }
    //
    // Where the holes are, measured rather than assumed:
    //   link 1  *(ped + 0x48)   dereferenced immediately, never tested;
    //   link 2  +0x30           the game's own `if (v40)` - a null here is LEGAL
    //                           and must not be reported;
    //   link 3  +0x2C8          tested on the crew-emblem branch, NOT tested on
    //                           the palette-name branch, where a null makes the
    //                           store land at 8*component + 184 - a near-null
    //                           write, which is the crash.
    //
    // YimMenu keys on component == 2 with link 3 null. That is kept: which of
    // the two branches runs is decided by a call we cannot cheaply predict, so
    // the report can be a false positive when the guarded branch would have run
    // - but blocking still loses nothing, because on that branch a null link 3
    // means the whole body is skipped anyway.
    //
    // A null link 1 is reported for every component, not just 2: the
    // dereference that faults happens before the component is looked at, so the
    // early return is safe on any value and can never block a call that would
    // have completed.
    //
    // The return value is dead: sub_6E13F0 returns the stack-guard word, and
    // sub_8ACC50 calls it eleven times in a row without reading rax.
    void invalid_decal_hook(uint64_t self, int a2) {
        if (should_report(filter_id::invalid_decal)) {
            uint32_t bad = 0;
            if (!self) {
                bad = 1;
            } else {
                const uint64_t p1 = *(uint64_t*)(self + PED_DRAW_HANDLER);
                if (!p1) {
                    bad = 2;
                } else {
                    const uint64_t p2 = *(uint64_t*)(p1 + DH_VAR_DATA);
                    if (p2 && a2 == 2 && *(uint64_t*)(p2 + VAR_PALETTE_SET) == 0)
                        bad = 3;
                }
            }
            if (bad) {
                report(filter_id::invalid_decal, -1, 0, (uint32_t)a2, bad);
                if (should_block(filter_id::invalid_decal))
                    return;
            }
        }
        PROT_CHAIN(g_decal, invalid_decal_fn, self, a2);
    }

    // Searchlight sweep-target position, f(rdi = Vec3V* out, rsi = CPed*)
    // @ 0x108F770, PC twin 0x7FF6FE92DC94 (RVA 0x84DC94).
    //
    //   r12 = *(qword*)(ped + 0x14B0);          // GetMyVehicle()
    //   if (r12) { ...find the searchlight...  }
    //   xmm0 = *(xmmword*)(r12 + 0x90);          // <- vehicle matrix position,
    //                                            //    OUTSIDE the null test
    //   if (CSyncedEntity::GetEntity(searchlight + 192) && found) { ... }
    //
    // Two unguarded dereferences, both confirmed on PS4:
    //   1. the vehicle. The `if (r12)` block only guards the searchlight
    //      search; the `vmovaps xmm0, [r12+90h]` that follows it is
    //      unconditional, so a ped with no vehicle faults at address 0x90.
    //   2. the searchlight. `sub_15509B0(rbx + 192)` is called BEFORE the
    //      `&& found` test, so with no searchlight it runs
    //      CSyncedEntity::GetEntity on address 192 and reads 200.
    //
    // So a report from this filter means "this call would have faulted".
    // Expect zero of them in normal play, in Log and in Enforce.
    //
    // Deliberate deviation from YimMenu and from this task's brief: YimMenu
    // calls the game's get_searchlight(CPed*) accessor from inside the hook.
    // On PS4 that accessor does not exist as a callable function - it is
    // inlined into the target - and its standalone sibling sub_12977D0 takes a
    // CVehicle, loops the weapon manager and makes a virtual GetType() call per
    // weapon. That is more than reading a field, so per the brief the fields are
    // read directly instead: the same chain, the same offsets, no call into the
    // game and no indirect branch through a pointer a hostile sync supplied.
    // "Is this weapon the searchlight" is answered by comparing the object's
    // vtable pointer with CSearchLight's, which is the same fact the virtual
    // call would have returned.
    //
    // The return value is dead: both call sites (0x108B27B, 0x129924A) overwrite
    // rax on the next instruction.
    void searchlight_hook(void* out, void* ped_ptr) {
        if (should_report(filter_id::searchlight)) {
            const uint64_t ped = (uint64_t)ped_ptr;
            uint32_t bad = 0;
            uint32_t count = 0;
            if (!ped) {
                bad = 1;
            } else {
                const uint64_t veh = *(uint64_t*)(ped + PED_MY_VEHICLE);
                if (!veh) {
                    bad = 2;
                } else {
                    const uint64_t mgr = *(uint64_t*)(veh + VEH_WEAPON_MGR);
                    if (!mgr) {
                        bad = 3;
                    } else {
                        const int n = *(int*)(mgr + MGR_WEAPON_COUNT);
                        count = (uint32_t)n;
                        if (n <= 0 || n > MGR_MAX_WEAPONS) {
                            bad = 4;
                        } else {
                            const uint64_t vt = rage::invoker::g_eboot_base + RVA_VT_CSEARCHLIGHT;
                            bool found = false;
                            for (int i = 0; i < n; i++) {
                                const uint64_t w =
                                    *(uint64_t*)(mgr + MGR_WEAPON_ARRAY + 8 * (uint64_t)i);
                                if (w && *(uint64_t*)w == vt) { found = true; break; }
                            }
                            if (!found) bad = 5;
                        }
                    }
                }
            }
            if (bad) {
                report(filter_id::searchlight, -1, 0, bad, count);
                if (should_block(filter_id::searchlight))
                    return;
            }
        }
        PROT_CHAIN(g_searchlight, searchlight_fn, out, ped_ptr);
    }
}

bool install_invalid_decal() { return install_detour(&g_decal,       RVA_INVALID_DECAL, (void*)&invalid_decal_hook); }
bool install_searchlight()   { return install_detour(&g_searchlight, RVA_SEARCHLIGHT,   (void*)&searchlight_hook); }
}
