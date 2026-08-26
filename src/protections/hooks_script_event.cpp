#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/learn.h"
#include "protections/players.h"

// CScriptedGameEvent::Decide - every incoming script event, with its sender.
//
// Evidence: analysis/PROTECTIONS_ANCHORS.md section 14 in the IDA repo
// (E:\Projects\IDA\PS4\GTA5). Every offset and the return convention below are
// read out of the PS4 disassembly and the leaked neteventmgr.cpp; nothing is
// ported from the PC build on faith.
//
// REPORT LEGEND
//   script_event: a = args[0] (the event type hash), b = arg count
//
// This filter ships in LEARN mode: it records the distinct set of event hashes
// it sees and blocks nothing by type. That is deliberate. YimMenu's eRemoteEvent
// constants are joaat hashes compiled into the freemode script, and the leaked
// dev branch, the PC build YimMenu targets, and PS4 1.57 are three different
// script revisions - a block list built from unverified constants would either
// do nothing, or drop legitimate script events and present as random mission
// failures. Building and testing that list against a real session is Task 14.
//
// What this filter DOES enforce is the per-player net_events block (Task 3),
// which needs no hash knowledge at all.
//
// Return convention (ANCHORS section 14, from neteventmgr.cpp:586-654): the
// dispatch runs `if (pEvent->Decide(...))` and, on TRUE, actions the event and
// acks it; on FALSE it neither actions nor acks (and a reliable event that is
// never acked is retransmitted). The apply lives inside Decide's own body, so a
// hook that returns true WITHOUT chaining to the original skips the effect and
// still gets the event acked - which is why the block path returns true.
namespace protections {
namespace {
    // CScriptedGameEvent::Decide - vtable slot 8 (0x40) of 0x30A8E80.
    // Prologue: whole-instruction boundary at exactly 14, register-only; the
    // RIP-relative load at offset 20 sits outside the steal. check_prologue over
    // the live bytes returns steal_len 14, SAFE. See ANCHORS section 14.
    const uint64_t RVA_SCRIPTED_GAME_EVENT_DECIDE = 0x16C5F10;

    // CScriptedGameEvent layout (read out of both Decide and Handle).
    const uint64_t EVT_ARGS       = 0x70;    // uint32_t[]
    const uint64_t EVT_ARG_COUNT  = 0x224;   // count, not bytes; max 0x1B0
    const uint32_t EVT_ARG_MAX    = 0x1B0;

    // CNetGamePlayer+0x31 is the physical player index (live-verified, Tier 1).
    const uint64_t NGP_PLAYER_IDX = 0x31;

    typedef bool (*scripted_decide_fn)(uint64_t, void*);

    detour_slot g_scripted;

    learn_table& learned() {
        static learn_table instance;   // function-local static: no .init_array
        return instance;
    }

    bool scripted_decide_hook(uint64_t self, void* from_player) {
        if (!should_report(filter_id::script_event))
            return PROT_CHAIN(g_scripted, scripted_decide_fn, self, from_player);

        const uint32_t count = *(const volatile uint32_t*)(self + EVT_ARG_COUNT);
        if (count == 0 || count > EVT_ARG_MAX)
            return PROT_CHAIN(g_scripted, scripted_decide_fn, self, from_player);

        const uint32_t hash = *(const volatile uint32_t*)(self + EVT_ARGS);

        int player = -1;
        if (from_player) {
            const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
            if (idx < 32) player = (int)idx;
        }

        // Report only the FIRST sighting of a hash. The ten-thousandth sighting
        // of a known one is noise, and the coalescer cannot help here: it keys
        // on filter_id, so it would collapse every distinct hash into one record.
        if (learned().observe(hash, player >= 0 ? (uint8_t)player : 0xFF))
            report(filter_id::script_event, player, 0, hash, count);

        // The one thing this filter enforces today. No hash knowledge needed.
        if (player >= 0 &&
            player_blocks().is_blocked(block_kind::net_events, player) &&
            should_block(filter_id::script_event)) {
            return true;   // handled; the event never reaches the scripts
        }

        return PROT_CHAIN(g_scripted, scripted_decide_fn, self, from_player);
    }
}

bool install_script_event() {
    return install_detour(&g_scripted, RVA_SCRIPTED_GAME_EVENT_DECIDE,
                          (void*)&scripted_decide_hook);
}

int  learned_count() { return learned().count(); }
bool learned_at(int i, uint32_t* hash, uint32_t* hits, uint8_t* first) {
    return learned().at(i, hash, hits, first);
}
void learned_clear() { learned().clear(); }
}
