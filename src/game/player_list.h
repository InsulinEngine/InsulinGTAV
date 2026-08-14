#pragma once
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

// The players in the current session.
//
// Ozark's whole Network branch hangs off a player manager; this is the equivalent,
// built purely from natives rather than by walking CNetworkPlayerMgr. That matters
// for two reasons: it needs no new RE, and it cannot go stale against a struct
// change. The engine path is documented in the RE catalog if raw access is ever
// needed (g_NetworkPlayerMgr 0x3080BF0, +0x108 local CNetGamePlayer, +0xB0
// CPlayerInfo, +0x280 CPed).
//
// The 32 bound is the game's own: CTheScripts::GetPlayerPedFromIndex rejects
// anything above 0x1F, which is visible in the decompiled native.
namespace game::players {

    static constexpr int MAX_PLAYERS = 32;

    struct entry {
        int         id;
        Ped         ped;
        const char* name;
        bool        alive;
    };

    inline bool in_session() {
        return native::network_is_session_active();
    }

    inline bool valid(int id) {
        return id >= 0 && id < MAX_PLAYERS && native::network_is_player_active(id);
    }

    inline entry get(int id) {
        entry e = { id, 0, "", false };
        if (!valid(id))
            return e;
        e.ped = native::get_player_ped(id);
        e.name = native::get_player_name(id);
        e.alive = e.ped && !native::is_player_dead(id);
        return e;
    }

    // Number of slots actually occupied. Story Mode reports 0 or 1 depending on
    // where the game is in its boot, which is why callers should check in_session()
    // rather than treating a non-zero count as "there are other players".
    inline int count() {
        int n = 0;
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (valid(i))
                n++;
        return n;
    }

    inline int local_id() {
        return native::player_id();
    }
}
