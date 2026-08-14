#include "menu/base/submenus/vehicle_collision.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    // Everything here acts on the vehicle you are sitting in. Nothing applies on
    // foot, so each entry resolves it first rather than assuming one is there.
    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }

    bool g_no_collision = false;
    bool g_latched = false;
}

void vehicle_collision_menu::load() {
    set_name("Collision");
    set_parent<vehicle_menu>();

    add_option(toggle_option("Disable Full Collision")
        .add_toggle(g_no_collision)
        .add_tooltip("Drive through everything")
        .add_savable(get_submenu_name_stack()));
}

void vehicle_collision_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (!v)
        return;

    if (g_no_collision) {
        native::set_entity_collision(v, false, false);
        g_latched = true;
    } else if (g_latched) {
        native::set_entity_collision(v, true, true);
        g_latched = false;
    }
}

vehicle_collision_menu* vehicle_collision_menu::get() {
    static vehicle_collision_menu instance;
    return &instance;
}
