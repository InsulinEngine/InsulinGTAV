#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/learn.h"
#include "protections/players.h"
#include "platform/log.h"

// Blast events. Only EXPLOSION_EVENT is hooked - it is the most-used remote kill.
// Its Decide (0x16BFFF0, vtable slot 8) is hooked; the other two in the group are
// identified-but-unhookable and carry null installers (see ANCHORS section 21):
//   ACTIVATE_VEHICLE_SPECIAL_ABILITY_EVENT (id 84) - Decide 0x16D3160 has a rel32
//     call at offset 13 inside the 14-byte steal; check_prologue refuses it.
//   KICK_VOTES_EVENT (id 64) - its Decide is the shared 3-byte stub 0x24787A0
//     (mov al,1; ret); nothing to hook, the vote tally is in Handle.
//
// SHIPS learn (explosion types) + per-player net_events block. explosion feeds a
// learn_table keyed on the explosion tag so the log shows the set of tags seen -
// the baseline the future in-range validity check needs - rather than a report
// per blast. The type-in-range and plausible-owner/position validity blocks are
// derived follow-ups; a guessed range would drop legitimate explosions.
//
// REPORT LEGEND
//   explosion: a = explosion tag (type), b = explosion owner object id
namespace protections {
namespace {
    // eboot RVA, CUSA00411 v1.57, imagebase 0. Decide = slot 8 of vtable 0x30A8580.
    const uint64_t RVA_EXPLOSION = 0x16BFFF0;   // id 17

    // Fields from the deserialiser 0x246F5F0, in the leak's SerialiseEvent order:
    // network id, 3 object ids (exploding/owner/ignore), then the tag.
    const uint64_t EXP_TAG        = 0x30;   // s32 m_explosionTag (8-bit on the wire)
    const uint64_t EXP_OWNER_ID   = 0xD4;   // u16 m_entExplosionOwnerID (13-bit)
    const uint64_t NGP_PLAYER_IDX = 0x31;   // CNetGamePlayer physical index

    typedef bool (*event_decide_fn)(uint64_t, void*, void*);

    detour_slot g_explosion;

    learn_table& explosion_tags() {
        static learn_table instance;   // function-local static: no .init_array
        return instance;
    }

    int sender_player(void* from_player) {
        if (!from_player) return -1;
        const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
        return idx < 32 ? (int)idx : -1;
    }

    bool explosion_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::explosion)) {
            int player = sender_player(from_player);

            if (player >= 0 &&
                player_blocks().is_blocked(block_kind::net_events, player) &&
                should_block(filter_id::explosion))
                return true;

            const uint32_t tag   = *(const volatile uint32_t*)(self + EXP_TAG);
            const uint32_t owner = *(const volatile uint16_t*)(self + EXP_OWNER_ID);
            if (explosion_tags().observe(tag, player >= 0 ? (uint8_t)player : 0xFF))
                report(filter_id::explosion, player, 0, tag, owner);
        }
        return PROT_CHAIN(g_explosion, event_decide_fn, self, from_player, to_player);
    }
}

bool install_explosion() {
    return install_detour(&g_explosion, RVA_EXPLOSION, (void*)&explosion_hook);
}

int  explosion_learned_count() { return explosion_tags().count(); }
void explosion_learned_clear() { explosion_tags().clear(); }

void explosion_learned_dump() {
    uint32_t tag, hits; uint8_t first;
    const int n = explosion_tags().count();
    platform::logf("Learn", "explosion dump: %d distinct", n);
    for (int i = 0; i < n; i++)
        if (explosion_tags().at(i, &tag, &hits, &first))
            platform::logf("Learn", "explosion %08x hits=%u p=%u", (unsigned)tag, (unsigned)hits, (unsigned)first);
}
}
