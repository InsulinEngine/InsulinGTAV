#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/players.h"

// Script state events - SCRIPT_ENTITY_STATE_CHANGE, hooked at Decide (vtable slot
// 8). Evidence: PROTECTIONS_ANCHORS.md section 18 in the IDA repo
// (E:\Projects\IDA\PS4\GTA5).
//
// SCRIPT_WORLD_STATE_EVENT (id 33) is IDENTIFIED BUT NOT HOOKED: its Decide
// (0x16C8090) is a 3-byte stub `mov al,1; ret` - it applies nothing (the effect,
// ChangeWorldState/RevertWorldState, lives in Handle 0x16C8040), and check_prologue
// refuses it because a 14-byte detour would run past the function's only
// instruction into the next one. Its registry row therefore has a null installer,
// exactly as fwBasePool::New does in Tier 1. See ANCHORS section 18.
//
// SHIPS the per-player net_events block plus a report of the targeted entity for
// SCRIPT_ENTITY_STATE. The out-of-range-state-type validity block (reject a
// stateChangeType outside the game's own switch range) is a derived follow-up -
// the discriminator lives in a polymorphic parameter sub-object, not yet mapped,
// and a guessed range would drop legitimate state changes.
//
// REPORT LEGEND
//   script_entity_state: a = target entity object id, b = 0
namespace protections {
namespace {
    // eboot RVA, CUSA00411 v1.57, imagebase 0. Decide = slot 8 of vtable 0x30A9FC0.
    const uint64_t RVA_SCRIPT_ENTITY_STATE = 0x16CDC70;   // id 50

    // m_ScriptEntityID: SERIALISE_OBJECTID (13-bit), read as movzx [r15+2Ch] in
    // the Decide itself.
    const uint64_t ENTITY_ID      = 0x2C;   // u16 object id
    const uint64_t NGP_PLAYER_IDX = 0x31;   // CNetGamePlayer physical index

    typedef bool (*event_decide_fn)(uint64_t, void*, void*);

    detour_slot g_script_entity_state;

    int sender_player(void* from_player) {
        if (!from_player) return -1;
        const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
        return idx < 32 ? (int)idx : -1;
    }

    bool script_entity_state_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::script_entity_state)) {
            int player = sender_player(from_player);

            if (player >= 0 &&
                player_blocks().is_blocked(block_kind::net_events, player) &&
                should_block(filter_id::script_entity_state))
                return true;

            const uint32_t entity = *(const volatile uint16_t*)(self + ENTITY_ID);
            report(filter_id::script_entity_state, player, 0, entity, 0);
        }
        return PROT_CHAIN(g_script_entity_state, event_decide_fn, self, from_player, to_player);
    }
}

bool install_script_entity_state() {
    return install_detour(&g_script_entity_state, RVA_SCRIPT_ENTITY_STATE,
                          (void*)&script_entity_state_hook);
}
}
