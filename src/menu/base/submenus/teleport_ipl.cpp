#include "menu/base/submenus/teleport_ipl.h"
#include "menu/base/submenus/teleport.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    // Move the vehicle when seated: pulling the ped out from under a car leaves
    // the car behind and drops you through the world.
    Entity subject() {
        Ped ped = native::get_player_ped(-1);
        if (ped && native::is_ped_in_any_vehicle(ped, false)) {
            Vehicle v = native::get_vehicle_ped_is_in(ped, false);
            if (v) return v;
        }
        return ped;
    }

    void warp(float x, float y, float z) {
        Entity e = subject();
        if (e) native::set_entity_coords_no_offset(e, x, y, z, false, false, false);
    }
    struct place { const char* name; float x, y, z; };
    const place k_places[] = {
        { "North Yankton",      3360.0f, -4849.0f, 112.6f },
        { "Carrier",            3082.0f, -4717.0f,  15.3f },
        { "Desert UFO",         2490.0f,  3774.0f, 2414.0f },
        { "Fort Zancudo UFO",  -2051.0f, 3237.0f, 1456.0f },
        { "Chiliad UFO",         501.0f, 5603.0f,  797.0f },
        { "Cluckin Bell",       -146.0f, 6161.0f,   31.0f },
        { "Eclipse Tower",      -773.0f,  312.0f,  187.0f },
        { "Maze Bank Roof",      -75.0f, -818.0f,  326.0f },
    };
    constexpr int PLACE_COUNT = (int)(sizeof(k_places) / sizeof(k_places[0]));
}

void teleport_ipl_menu::load() {
    set_name("IPL");
    set_parent<teleport_menu>();

    for (int i = 0; i < PLACE_COUNT; i++) {
        int idx = i;   // tiny capture: stl::function caps captures at 64 bytes
        add_option(button_option(k_places[i].name)
            .add_click([idx] {
                warp(k_places[idx].x, k_places[idx].y, k_places[idx].z);
                menu::notify::stacked("Teleport", k_places[idx].name);
            }));
    }
}

teleport_ipl_menu* teleport_ipl_menu::get() {
    static teleport_ipl_menu instance;
    return &instance;
}
