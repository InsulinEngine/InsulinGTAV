#include "menu/base/submenus/vehicle_movement.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/submenus/vehicle_acrobatics.h"
#include "menu/base/submenus/vehicle_parachute.h"
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

    bool g_bypass_max_speed = false;
}

void vehicle_movement_menu::load() {
    set_name("Movement");
    set_parent<vehicle_menu>();

    add_option(submenu_option("Acrobatics").add_submenu<vehicle_acrobatics_menu>());
    add_option(submenu_option("Parachute").add_submenu<vehicle_parachute_menu>());

    add_option(toggle_option("Bypass Max Speed")
        .add_toggle(g_bypass_max_speed)
        .add_tooltip("Lifts the speed limiter while you hold the throttle")
        .add_savable(get_submenu_name_stack()));
}

void vehicle_movement_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (v && g_bypass_max_speed)
        native::set_vehicle_max_speed(v, 0.f);
}

vehicle_movement_menu* vehicle_movement_menu::get() {
    static vehicle_movement_menu instance;
    return &instance;
}
