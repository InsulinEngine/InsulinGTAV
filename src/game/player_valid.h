#pragma once
#include "platform/stdafx.h"
#include "rage/invoker/invoker.h"

// Is the game far enough along that touching it is safe?
//
// Ozark solves this by waiting for its g_game_state to reach GameStatePlaying before
// it initialises anything (init.cpp: `while (*g_game_state != GameStatePlaying)
// Sleep(500);`). This port never had that gate, which is why a saved Godmode toggle
// could take the game down on boot: the plugin loads while the game may still be on
// its loading screen, and a native like GET_PLAYER_PED then walks a player that does
// not exist yet. Whether it crashed depended on how far the game had booted, so it
// looked random.
//
// Rather than resolve a game-state global, this checks the thing we actually care
// about and already have a verified anchor for: the local player ped. It is pure
// memory reads - no natives - so it is safe to call from anywhere, including before
// the game is up.
//
//   g_CPedFactory (RVA 0x3278708) holds a CPedFactory*  (deref once)
//   CPedFactory + 0x08            holds the local player CPed*
//
// Both proven on-console; see the PS4 RE catalog.
namespace game {

    static constexpr uint64_t RVA_PED_FACTORY = 0x3278708;
    static constexpr uint32_t FACTORY_LOCAL_PED_OFF = 0x08;

    inline bool player_valid() {
        uintptr_t base = rage::invoker::g_eboot_base;
        if (!base)
            return false;

        uintptr_t factory = *(uintptr_t*)(base + RVA_PED_FACTORY);
        if (!factory)
            return false;

        return *(uintptr_t*)(factory + FACTORY_LOCAL_PED_OFF) != 0;
    }
}
