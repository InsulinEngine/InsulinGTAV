#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/players.h"

// Ped task events - NETWORK_CLEAR_PED_TASKS / RAGDOLL_REQUEST, hooked at each
// event class's Decide (vtable slot 8). Both are direct griefing tools:
// clear-tasks interrupts whatever the target ped is doing, ragdoll-request knocks
// it down remotely. Evidence: PROTECTIONS_ANCHORS.md section 17 in the IDA repo
// (E:\Projects\IDA\PS4\GTA5).
//
// SHIPS the per-player net_events block plus a report of each event; no
// learn_table (the reported value is a transient net object id). The
// targets-the-local-player validity block (refuse when m_pedId is the local
// player's ped net object and the sender has no business acting on it) is a
// derived follow-up - it needs m_pedId compared against the local ped net object
// (netObject+0x0A), and a mis-derived compare would drop legitimate task syncs.
// Not guessed; the per-player block covers the targeted-griefer case meanwhile.
//
// REPORT LEGEND
//   clear_ped_tasks: a = target ped object id, b = 0
//   ragdoll_request: a = target ped object id, b = 0
namespace protections {
namespace {
    // eboot RVAs, CUSA00411 v1.57, imagebase 0. Decide = slot 8 of each vtable.
    const uint64_t RVA_CLEAR_PED_TASKS = 0x16CB360;   // vtable 0x30A99C0 (id 43)
    const uint64_t RVA_RAGDOLL_REQUEST = 0x16C3FB0;   // vtable 0x30A8AC0 (id 24)

    // Both events carry a single 13-bit m_pedId, read into +0x2C by their Handle
    // (0x16CB2C0 / 0x16C3F20) and again in each Decide (movzx [r14+2Ch]).
    const uint64_t PED_ID         = 0x2C;   // u16 object id
    const uint64_t NGP_PLAYER_IDX = 0x31;   // CNetGamePlayer physical index

    typedef bool (*event_decide_fn)(uint64_t, void*, void*);

    detour_slot g_clear_ped_tasks;
    detour_slot g_ragdoll_request;

    int sender_player(void* from_player) {
        if (!from_player) return -1;
        const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
        return idx < 32 ? (int)idx : -1;
    }

    bool handle(filter_id id, void* from_player, uint32_t ped_id) {
        int player = sender_player(from_player);

        if (player >= 0 &&
            player_blocks().is_blocked(block_kind::net_events, player) &&
            should_block(id))
            return true;

        report(id, player, 0, ped_id, 0);
        return false;
    }

    bool clear_ped_tasks_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::clear_ped_tasks)) {
            const uint32_t ped = *(const volatile uint16_t*)(self + PED_ID);
            if (handle(filter_id::clear_ped_tasks, from_player, ped))
                return true;
        }
        return PROT_CHAIN(g_clear_ped_tasks, event_decide_fn, self, from_player, to_player);
    }

    bool ragdoll_request_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::ragdoll_request)) {
            const uint32_t ped = *(const volatile uint16_t*)(self + PED_ID);
            if (handle(filter_id::ragdoll_request, from_player, ped))
                return true;
        }
        return PROT_CHAIN(g_ragdoll_request, event_decide_fn, self, from_player, to_player);
    }
}

bool install_clear_ped_tasks() { return install_detour(&g_clear_ped_tasks, RVA_CLEAR_PED_TASKS, (void*)&clear_ped_tasks_hook); }
bool install_ragdoll_request() { return install_detour(&g_ragdoll_request, RVA_RAGDOLL_REQUEST, (void*)&ragdoll_request_hook); }
}
