#pragma once

#include "game/teleport_fsm.h"
#include "rage/types/base_types.h"

// The engine behind game::tp::machine. Everything here calls natives, so it is
// game-thread only and deliberately kept out of teleport_fsm.h, which compiles
// for the host.

namespace game {

    // Move the vehicle when seated, not the ped: pulling the ped out from under
    // a car leaves the car behind and drops the player through the world.
    Entity teleport_subject();

    // The real actions table. Safe to call before the hash natives are up; the
    // ground query simply reports "not yet", which the machine already handles.
    const tp::actions& live_actions();

    // Is GET_GROUND_Z_FOR_3D_COORD actually callable yet? It is a hash native,
    // so it is inert until the command table has been recovered and verified.
    // While it is inert every rung of the sweep reports "no ground" and the
    // give-up log looks exactly like a streaming failure - a different bug
    // with a different fix. Logged once per teleport so a console run can
    // never confuse the two.
    bool ground_native_ready();
}
