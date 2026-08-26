#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/learn.h"
#include "protections/players.h"
#include "platform/log.h"

// Weapon events - WEAPON_DAMAGE / GIVE_WEAPON / REMOVE_WEAPON, hooked at each
// event class's Decide (vtable slot 8). Evidence: PROTECTIONS_ANCHORS.md section
// 15 in the IDA repo (E:\Projects\IDA\PS4\GTA5). Every offset is read out of the
// PS4 deserialiser and cross-checked against the leaked NetworkEventTypes.cpp
// SerialiseEvent order; nothing is ported from YimMenu.
//
// SHIPS LEARN + PER-PLAYER BLOCK, no per-event validity yet. The valuable check
// is weapon-hash validity - reject a damage event whose weapon hash is not a
// real CWeaponInfo. That needs CWeaponInfoManager::GetInfo (NetworkEventTypes.cpp
// :1743) and the g_WeaponInfoArray layout, which has no direct xref and must be
// derived before it can be trusted. Until then these filters record the distinct
// set of weapon hashes seen (the exact baseline that validity check needs) and
// enforce only the per-player net_events block. Blocking a good weapon hash would
// drop legitimate damage, so it is deliberately not guessed.
//
// REPORT LEGEND
//   weapon_damage: a = weapon hash, b = damage type (0..3)
//   give_weapon:   a = weapon hash, b = target ped object id
//   remove_weapon: a = weapon hash, b = target ped object id
//
// Return convention is CScriptedGameEvent::Decide's (ANCHORS section 14): the
// dispatch actions the event iff Decide returns true, so the per-player block
// returns true WITHOUT chaining - the apply in the original body never runs and
// the event is still acked. Decide takes (this, fromPlayer, toPlayer); WEAPON_
// DAMAGE reads toPlayer, so every hook forwards all three - dropping the third
// would hand the original a corrupt toPlayer on the passthrough path.
namespace protections {
namespace {
    // eboot RVAs, CUSA00411 v1.57, imagebase 0. Decide = slot 8 of each vtable.
    const uint64_t RVA_WEAPON_DAMAGE = 0x16B9AF0;   // vtable 0x30A7EC0
    const uint64_t RVA_GIVE_WEAPON   = 0x16BCAB0;   // vtable 0x30A81C0
    const uint64_t RVA_REMOVE_WEAPON = 0x16BCF10;   // vtable 0x30A8280

    // Field offsets, read out of each event's SerialiseEvent (Handle thunks:
    // 0x246C9E0 / 0x246DC10 / 0x246DDD0).
    const uint64_t WD_WEAPON_HASH = 0x54;   // u32, after the 2-bit damage type
    const uint64_t WD_DAMAGE_TYPE = 0x80;   // u8, 0..3
    const uint64_t GW_WEAPON_HASH = 0x30;   // u32
    const uint64_t GW_PED_ID      = 0x2C;   // u16 object id
    // REMOVE_WEAPON shares GIVE_WEAPON's +0x2C/+0x30 layout.

    const uint64_t NGP_PLAYER_IDX = 0x31;   // CNetGamePlayer physical index

    typedef bool (*event_decide_fn)(uint64_t, void*, void*);

    detour_slot g_weapon_damage;
    detour_slot g_give_weapon;
    detour_slot g_remove_weapon;

    // One table for the union of weapon hashes seen across all three events -
    // exactly the set the future validity check must accept.
    learn_table& weapon_hashes() {
        static learn_table instance;   // function-local static: no .init_array
        return instance;
    }

    int sender_player(void* from_player) {
        if (!from_player) return -1;
        const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
        return idx < 32 ? (int)idx : -1;
    }

    // Common learn + per-player-block body. Returns true if the caller should
    // BLOCK (return true without chaining), false to chain to the original.
    bool handle(filter_id id, void* from_player,
                uint32_t hash, uint32_t detail_b) {
        int player = sender_player(from_player);

        if (player >= 0 &&
            player_blocks().is_blocked(block_kind::net_events, player) &&
            should_block(id))
            return true;

        // Learn: report only the first sighting of each distinct weapon hash.
        if (weapon_hashes().observe(hash, player >= 0 ? (uint8_t)player : 0xFF))
            report(id, player, 0, hash, detail_b);

        return false;
    }

    bool weapon_damage_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::weapon_damage)) {
            const uint32_t hash = *(const volatile uint32_t*)(self + WD_WEAPON_HASH);
            const uint32_t type = *(const volatile uint8_t*)(self + WD_DAMAGE_TYPE);
            if (handle(filter_id::weapon_damage, from_player, hash, type))
                return true;
        }
        return PROT_CHAIN(g_weapon_damage, event_decide_fn, self, from_player, to_player);
    }

    bool give_weapon_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::give_weapon)) {
            const uint32_t hash = *(const volatile uint32_t*)(self + GW_WEAPON_HASH);
            const uint32_t ped  = *(const volatile uint16_t*)(self + GW_PED_ID);
            if (handle(filter_id::give_weapon, from_player, hash, ped))
                return true;
        }
        return PROT_CHAIN(g_give_weapon, event_decide_fn, self, from_player, to_player);
    }

    bool remove_weapon_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::remove_weapon)) {
            const uint32_t hash = *(const volatile uint32_t*)(self + GW_WEAPON_HASH);
            const uint32_t ped  = *(const volatile uint16_t*)(self + GW_PED_ID);
            if (handle(filter_id::remove_weapon, from_player, hash, ped))
                return true;
        }
        return PROT_CHAIN(g_remove_weapon, event_decide_fn, self, from_player, to_player);
    }
}

bool install_weapon_damage() { return install_detour(&g_weapon_damage, RVA_WEAPON_DAMAGE, (void*)&weapon_damage_hook); }
bool install_give_weapon()   { return install_detour(&g_give_weapon,   RVA_GIVE_WEAPON,   (void*)&give_weapon_hook); }
bool install_remove_weapon() { return install_detour(&g_remove_weapon, RVA_REMOVE_WEAPON, (void*)&remove_weapon_hook); }

int  weapon_learned_count() { return weapon_hashes().count(); }
void weapon_learned_clear() { weapon_hashes().clear(); }

void weapon_learned_dump() {
    uint32_t hash, hits; uint8_t first;
    const int n = weapon_hashes().count();
    platform::logf("Learn", "weapon dump: %d distinct", n);
    for (int i = 0; i < n; i++)
        if (weapon_hashes().at(i, &hash, &hits, &first))
            platform::logf("Learn", "weapon %08x hits=%u p=%u", (unsigned)hash, (unsigned)hits, (unsigned)first);
}
}
