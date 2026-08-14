#include "menu/base/submenus/teleport_save_load.h"
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
    bool  g_have = false;
    float g_x = 0.f, g_y = 0.f, g_z = 0.f;
}

void teleport_save_load_menu::load() {
    set_name("Save and Load");
    set_parent<teleport_menu>();

    add_option(button_option("Save Current Position")
        .add_click([] {
            Entity e = subject();
            if (!e) return;
            math::vector3<float> c = native::get_entity_coords(e, true);
            g_x = c.x; g_y = c.y; g_z = c.z; g_have = true;
            menu::notify::stacked("Teleport", "Position saved");
        }));

    add_option(button_option("Load Saved Position")
        .add_requirement([] { return g_have; })
        .add_click([] { warp(g_x, g_y, g_z); }));
}

teleport_save_load_menu* teleport_save_load_menu::get() {
    static teleport_save_load_menu instance;
    return &instance;
}
