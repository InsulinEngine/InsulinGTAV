#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/learn.h"
#include "protections/players.h"

// Nuisance events - NETWORK_PLAY_SOUND / CHANGE_RADIO_STATION / DOOR_BREAK, hooked
// at each event's Decide (vtable slot 8). Sound spam is the highest-volume attack
// in the whole set, which makes it the best real-world test of the report ring's
// coalescer. Evidence: PROTECTIONS_ANCHORS.md section 19 in the IDA repo.
//
// SHIPS learn (sound hashes) + per-player net_events block. play_sound feeds a
// learn_table so a spam burst reports one new hash rather than a flood; radio and
// door are low-volume and report each occurrence (coalesced). The rate-limit /
// vehicle-ownership validity blocks are derived follow-ups, not guessed.
//
// REPORT LEGEND
//   play_sound:   a = sound name hash, b = secondary sound field (0x40)
//   change_radio: a = target vehicle object id, b = station id
//   door_break:   a = target vehicle object id, b = door index
namespace protections {
namespace {
    // eboot RVAs, CUSA00411 v1.57, imagebase 0. Decide = slot 8 of each vtable.
    const uint64_t RVA_PLAY_SOUND   = 0x16CE8E0;   // vtable 0x30AA080 (id 51)
    const uint64_t RVA_CHANGE_RADIO = 0x16C3C30;   // vtable 0x30A8A00 (id 23)
    const uint64_t RVA_DOOR_BREAK   = 0x16C5520;   // vtable 0x30A8DC0 (id 27)

    // Field offsets from each deserialiser.
    const uint64_t PS_SOUND_HASH  = 0x44;   // u32, unconditional; candidate sound name hash
    const uint64_t PS_SECOND      = 0x40;   // u32, conditional secondary sound field
    const uint64_t VEH_ID         = 0x2C;   // u16 m_vehicleId (radio + door; Decide movzx [reg+2Ch])
    const uint64_t RADIO_STATION  = 0x2E;   // u8 m_stationId
    const uint64_t DOOR_INDEX     = 0x2E;   // u8 m_door

    const uint64_t NGP_PLAYER_IDX = 0x31;   // CNetGamePlayer physical index

    typedef bool (*event_decide_fn)(uint64_t, void*, void*);

    detour_slot g_play_sound;
    detour_slot g_change_radio;
    detour_slot g_door_break;

    learn_table& sound_hashes() {
        static learn_table instance;   // function-local static: no .init_array
        return instance;
    }

    int sender_player(void* from_player) {
        if (!from_player) return -1;
        const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
        return idx < 32 ? (int)idx : -1;
    }

    // Per-player block. Returns true if the caller should BLOCK.
    bool blocked(filter_id id, int player) {
        return player >= 0 &&
               player_blocks().is_blocked(block_kind::net_events, player) &&
               should_block(id);
    }

    bool play_sound_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::play_sound)) {
            int player = sender_player(from_player);
            if (blocked(filter_id::play_sound, player)) return true;

            const uint32_t hash = *(const volatile uint32_t*)(self + PS_SOUND_HASH);
            const uint32_t two  = *(const volatile uint32_t*)(self + PS_SECOND);
            // Learn: one report per distinct sound hash, so spam shows as a
            // growing set rather than a flood.
            if (sound_hashes().observe(hash, player >= 0 ? (uint8_t)player : 0xFF))
                report(filter_id::play_sound, player, 0, hash, two);
        }
        return PROT_CHAIN(g_play_sound, event_decide_fn, self, from_player, to_player);
    }

    bool change_radio_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::change_radio)) {
            int player = sender_player(from_player);
            if (blocked(filter_id::change_radio, player)) return true;

            const uint32_t veh = *(const volatile uint16_t*)(self + VEH_ID);
            const uint32_t st  = *(const volatile uint8_t*)(self + RADIO_STATION);
            report(filter_id::change_radio, player, 0, veh, st);
        }
        return PROT_CHAIN(g_change_radio, event_decide_fn, self, from_player, to_player);
    }

    bool door_break_hook(uint64_t self, void* from_player, void* to_player) {
        if (should_report(filter_id::door_break)) {
            int player = sender_player(from_player);
            if (blocked(filter_id::door_break, player)) return true;

            const uint32_t veh  = *(const volatile uint16_t*)(self + VEH_ID);
            const uint32_t door = *(const volatile uint8_t*)(self + DOOR_INDEX);
            report(filter_id::door_break, player, 0, veh, door);
        }
        return PROT_CHAIN(g_door_break, event_decide_fn, self, from_player, to_player);
    }
}

bool install_play_sound()   { return install_detour(&g_play_sound,   RVA_PLAY_SOUND,   (void*)&play_sound_hook); }
bool install_change_radio() { return install_detour(&g_change_radio, RVA_CHANGE_RADIO, (void*)&change_radio_hook); }
bool install_door_break()   { return install_detour(&g_door_break,   RVA_DOOR_BREAK,   (void*)&door_break_hook); }

int  sound_learned_count() { return sound_hashes().count(); }
void sound_learned_clear() { sound_hashes().clear(); }
}
