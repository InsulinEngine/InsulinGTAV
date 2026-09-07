#pragma once
#include "platform/stdafx.h"
#include "rage/invoker/invoker.h"

#include <math.h>

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

    // The local player CPed*, or 0. Pure memory reads - the chain described
    // above, kept in one place so player_valid() and the readers below cannot
    // drift apart.
    inline uintptr_t local_player_ped() {
        uintptr_t base = rage::invoker::g_eboot_base;
        if (!base)
            return 0;

        uintptr_t factory = *(uintptr_t*)(base + RVA_PED_FACTORY);
        if (!factory)
            return 0;

        return *(uintptr_t*)(factory + FACTORY_LOCAL_PED_OFF);
    }

    inline bool player_valid() {
        return local_player_ped() != 0;
    }

    // Matrix34 on CEntity at +0x60: right row +0x60, forward row +0x70, up row
    // +0x80, position row +0x90 (RE catalog section 7). As floats from +0x60
    // that is m[0..2] right, m[4..6] forward, m[12..14] position. Confirmed
    // live: a standing ped reads right=(-1,-0.02,0), forward=(0.02,-1,0),
    // up=(0,0,1).
    //
    // The indirection matters. NAT_GET_ENTITY_HEADING (0x9B2DF0) takes the
    // matrix from the entity at +0x14B0 instead when the ped is type 4 and
    // carries config flag bit 0x40 at +0x13CB (InVehicle) - and it requires
    // both, the flag and a non-null pointer. Without this the marker freezes
    // at the spot where you got into the car, which reads exactly like a
    // broken map rather than a missing dereference.
    inline const float* local_player_matrix() {
        uintptr_t ped = local_player_ped();
        if (!ped)
            return 0;

        uintptr_t src = ped;
        if (*(const volatile unsigned char*)(ped + 0x13CB) & 0x40) {
            uintptr_t veh = *(const volatile uintptr_t*)(ped + 0x14B0);
            if (veh)
                src = veh;
        }
        return (const float*)(src + 0x60);
    }

    // Degrees in [0, 360), byte-for-byte the same computation as
    // GET_ENTITY_HEADING: atan2f of +0x64 over +0x74 - right.y over forward.y -
    // scaled by 57.29578 and wrapped.
    //
    // NOT atan2f(-forward.x, forward.y). The two agree for anything level,
    // because right.y == -forward.x holds for a pure heading rotation, and they
    // disagree the moment the matrix carries roll or pitch - which is precisely
    // the +0x14B0 case above, where the matrix belongs to a banking vehicle.
    inline float local_player_heading() {
        const float* m = local_player_matrix();
        if (!m)
            return 0.0f;

        float deg = atan2f(m[1], m[5]) * 57.29578f;
        if (deg < 0.0f)
            deg += 360.0f;
        if (deg > 360.0f)
            deg -= 360.0f;
        return deg;
    }
}
