#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"

// Task guards. Both targets are CTask::UpdateFSM(this, iState, iEvent)
// overrides that dereference a pointer a hostile remote sync can leave null.
// Each guard is a null check on the exact chain the original walks - nothing
// more, so the guard cannot be wrong about a case the original handles.
//
// Evidence lives in analysis/PROTECTIONS_ANCHORS.md in the IDA repo
// (E:\Projects\IDA\PS4\GTA5), sections 6 and 7. Summary:
//
// The FSM event encoding was read out of the PS4 code, not assumed:
//   sub_E40A20 case 0 dispatches a3==0 to the body of Start_OnEnter
//   (pObject->m_nDEflags.bForcePrePhysicsAnimUpdate = true) and a3==1 to the
//   body of Start_OnUpdate (TaskSetState(State_Stream)). So on this build
//   OnEnter = 0, OnUpdate = 1, OnExit = 2, and YimMenu's (1,1) key means
//   iState = State_Stream, iEvent = OnUpdate.
//
// Every PC offset in the reference was re-derived here and all of them held;
// see the per-hook comments. That is not the same as assuming they would - the
// draw-list guard in hooks_render.cpp found a PC constant that did NOT port.
namespace protections {
namespace {
    // eboot RVAs, CUSA00411 v1.57, imagebase 0.
    const uint64_t RVA_TASK_AMBIENT_CLIPS = 0xD08A90;  // CTaskAmbientClips::UpdateFSM
    const uint64_t RVA_TASK_PARACHUTE     = 0xE40A20;  // CTaskParachuteObject::UpdateFSM

    typedef int (*task_update_fn)(uint64_t, int, int);

    detour_slot g_ambient;
    detour_slot g_parachute;

    // CTaskAmbientClips::UpdateFSM @ 0xD08A90, PC twin sub_7FF6FEBECEBC.
    //
    // this+0x100 is a qword pointer field of CTaskAmbientClips - the conditional
    // anims group the task plays from. A clone task is built from
    // CClonedTaskAmbientClipsInfo, whose serialised payload carries an attacker
    // controlled group hash plus a chosen-anim index; a hash that resolves to
    // nothing leaves this pointer null and the index still set.
    //
    // 0x100 is NOT ported on faith: PS4 CTaskAmbientClips__Start_OnUpdate
    // (0xD08E50) and its PC twin (0x7FF6FEBE43E4) both read *(qword*)(this+256),
    // and every other field the two functions touch - 264, 492, 516, 523, 168 -
    // is at the same offset in both builds. CTaskAmbientClips is not shifted
    // between PC and PS4 (CPed is: 4320 -> 4208 in the same pair of functions).
    //
    // KNOWN IMPRECISION, ported deliberately. The game itself treats a null
    // group as legal - Start_OnUpdate null-checks this exact field before use -
    // so this filter is expected to report during ordinary single-player play.
    // YimMenu says of its own version that it "doesn't block the crash
    // completely"; the brief for this task requires porting it as-is rather
    // than inventing a narrower condition. Consequence: reports here do not
    // imply a wrong offset, and this filter should stay on Log.
    int task_ambient_clips_hook(uint64_t self, int a2, int a3) {
        if (should_report(filter_id::task_ambient_clips) &&
            *(uint64_t*)(self + 0x100) == 0) {
            report(filter_id::task_ambient_clips, -1, 0, (uint32_t)a2, (uint32_t)a3);
            if (should_block(filter_id::task_ambient_clips))
                return 0;
        }
        return PROT_CHAIN(g_ambient, task_update_fn, self, a2, a3);
    }

    // CTaskParachuteObject::UpdateFSM @ 0xE40A20.
    //
    // Only iState == State_Stream (1) with iEvent == OnUpdate (1) reaches the
    // crashing code. That path is Stream_OnUpdate, sub_E40DC0, which does:
    //
    //     v6 = *(qword*)(this + 16);      // GetObject()      <- this+0x10
    //     v7 = *(qword*)(v6   + 80);      // anim director    <- +0x50
    //     if (v7) { if (*(qword*)(v7 + 64)) {   // move object <- +0x40
    //         ... CreateNetworkPlayer / SetClipSet / SetNetwork ...
    //
    // All three PC offsets held on PS4 and were read straight out of the PS4
    // target's own callee. Note where the hole is: the game already checks the
    // second and third links, and never checks the first - *(this+0x10) is
    // dereferenced unconditionally. Two more methods of the same class walk the
    // identical chain (CleanUp at 0xE40750 dereferences *(this+0x10) with no
    // check at all; UpdateFSM's own State_WaitForDeploy case walks +0x10 ->
    // +0x50 -> +0x48), which is what pins these offsets.
    //
    // Returning 0 on block is what the original returns on this exact path:
    // "case 1: if (a3 == 1) Stream_OnUpdate(this); return 0;" - 0 is
    // FSM_Continue, so a blocked frame is indistinguishable from a frame where
    // streaming had not finished yet.
    int task_parachute_hook(uint64_t self, int a2, int a3) {
        if (a2 == 1 && a3 == 1 && should_report(filter_id::task_parachute)) {
            uint64_t p1 = *(uint64_t*)(self + 0x10);
            uint64_t p2 = p1 ? *(uint64_t*)(p1 + 0x50) : 0;
            uint64_t p3 = p2 ? *(uint64_t*)(p2 + 0x40) : 0;
            if (!p3) {
                report(filter_id::task_parachute, -1, 0, (uint32_t)(p1 != 0), (uint32_t)(p2 != 0));
                if (should_block(filter_id::task_parachute))
                    return 0;
            }
        }
        return PROT_CHAIN(g_parachute, task_update_fn, self, a2, a3);
    }
}

bool install_task_ambient_clips() { return install_detour(&g_ambient,   RVA_TASK_AMBIENT_CLIPS, (void*)&task_ambient_clips_hook); }
bool install_task_parachute()     { return install_detour(&g_parachute, RVA_TASK_PARACHUTE,     (void*)&task_parachute_hook); }
}
