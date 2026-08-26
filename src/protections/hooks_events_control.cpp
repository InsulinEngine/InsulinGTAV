#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/players.h"

// Control events - REQUEST_CONTROL / GIVE_CONTROL, hooked at each event class's
// Decide (vtable slot 8). These are how an attacker takes ownership of your
// vehicle or ped before acting on it. Evidence: PROTECTIONS_ANCHORS.md section 16
// in the IDA repo (E:\Projects\IDA\PS4\GTA5).
//
// SHIPS the per-player net_events block plus a plain report of each event. Unlike
// the weapon filters there is no learn_table: the reported value is a net object
// id, which is a transient per-session handle, not a bounded set worth learning.
// The ownership-validity block (refuse a hand-over of an object you own and are
// using) is a derived follow-up - it needs the object-id compared against the
// local player's ped/vehicle net objects (CPed+0xD0 = m_pNetObj, netObject+0x0A =
// object id, both live-verified in Tier 1), and blocking on a mis-derived compare
// would break legitimate object migration. Not guessed.
//
// REPORT LEGEND
//   request_control: a = requested object id, b = 0
//   give_control:    a = number of objects handed over, b = first object's data word
//
// A PLAN CORRECTION recorded in section 16: the Tier 2 plan put GIVE_CONTROL on
// vtable 0x30A7D40. The factories disagree - factory 0x16B6120 (event id 4,
// REQUEST_CONTROL) stamps 0x30A7D40, and factory 0x16B6E90 (id 5, GIVE_CONTROL)
// stamps 0x30A7E00. The single-object-id deserialiser on 0x30A7D40 matches
// REQUEST_CONTROL's wire layout, confirming the swap. The IDB's "CGiveControlEvent"
// labels on 0x30A7D40 are PC-sig-port misnomers.
namespace protections {
namespace {
    // eboot RVAs, CUSA00411 v1.57, imagebase 0. Decide = slot 8 of each vtable.
    const uint64_t RVA_REQUEST_CONTROL = 0x16B63D0;   // vtable 0x30A7D40 (id 4)
    const uint64_t RVA_GIVE_CONTROL    = 0x16B7480;   // vtable 0x30A7E00 (id 5)

    // Field offsets, read out of each event's deserialiser.
    const uint64_t RC_OBJECT_ID   = 0x2C;   // u16 object id (single); movzx [r15+2Ch] in Decide
    const uint64_t GC_NUM_DATA    = 0x48;   // u32 m_numControlData (clear-loop bound in Handle)
    const uint64_t GC_FIRST_DATA  = 0x5C;   // first control-data entry word (stride 20 from here)

    const uint64_t NGP_PLAYER_IDX = 0x31;   // CNetGamePlayer physical index

    typedef bool (*event_decide_fn)(uint64_t, void*, void*);

    detour_slot g_request_control;
    detour_slot g_give_control;

    int sender_player(void* from_player) {
        if (!from_player) return -1;
        const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
        return idx < 32 ? (int)idx : -1;
    }

    // Per-player block + report. Returns true if the caller should BLOCK.
    bool handle(filter_id id, void* from_player, uint32_t a, uint32_t b) {
        int player = sender_player(from_player);

        if (player >= 0 &&
            player_blocks().is_blocked(block_kind::net_events, player) &&
            should_block(id))
            return true;

        report(id, player, 0, a, b);
        return false;
    }

    bool request_control_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::request_control)) {
            const uint32_t obj = *(const volatile uint16_t*)(self + RC_OBJECT_ID);
            if (handle(filter_id::request_control, from_player, obj, 0))
                return true;
        }
        return PROT_CHAIN(g_request_control, event_decide_fn, self, from_player, to_player);
    }

    bool give_control_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::give_control)) {
            const uint32_t n     = *(const volatile uint32_t*)(self + GC_NUM_DATA);
            const uint32_t first = *(const volatile uint32_t*)(self + GC_FIRST_DATA);
            if (handle(filter_id::give_control, from_player, n, first))
                return true;
        }
        return PROT_CHAIN(g_give_control, event_decide_fn, self, from_player, to_player);
    }
}

bool install_request_control() { return install_detour(&g_request_control, RVA_REQUEST_CONTROL, (void*)&request_control_hook); }
bool install_give_control()    { return install_detour(&g_give_control,    RVA_GIVE_CONTROL,    (void*)&give_control_hook); }
}
