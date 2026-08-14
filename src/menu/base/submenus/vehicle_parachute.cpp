#include "menu/base/submenus/vehicle_parachute.h"
#include "menu/base/submenus/vehicle_movement.h"
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
    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }
    bool g_auto_deploy = false;
}

void vehicle_parachute_menu::load() {
    set_name("Parachute");
    set_parent<vehicle_movement_menu>();

    add_option(toggle_option("Auto Deploy")
        .add_toggle(g_auto_deploy)
        .add_tooltip("Opens the vehicle parachute once you are falling")
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Deploy Now")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            // Only some vehicles carry one, and asking a car to deploy does
            // nothing at all - saying so beats a button that silently no-ops.
            if (!native::get_vehicle_has_parachute(v)) {
                menu::notify::stacked("Parachute", "This vehicle has none");
                return;
            }
            native::vehicle_start_parachuting(v, true);
        }));
}

void vehicle_parachute_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (!v || !g_auto_deploy)
        return;

    // Only once actually falling: deploying on the ground does nothing and
    // re-arming it every frame would stop it ever opening.
    if (native::is_entity_in_air(v) &&
        native::get_vehicle_has_parachute(v) &&
        !native::is_vehicle_parachute_deployed(v))
        native::vehicle_start_parachuting(v, true);
}

vehicle_parachute_menu* vehicle_parachute_menu::get() {
    static vehicle_parachute_menu instance;
    return &instance;
}
