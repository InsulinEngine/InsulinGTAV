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
}
