#include "menu/base/submenus/vehicle_weapons.h"
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

    bool g_weapons = false;
}

void vehicle_weapons_menu::load() {
    set_name("Weapons");
    set_parent<vehicle_menu>();

    add_option(toggle_option("Toggle Weapons")
        .add_toggle(g_weapons)
        .add_tooltip("Keeps the vehicle weapon enabled")
        .add_savable(get_submenu_name_stack()));
}

void vehicle_weapons_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (g_weapons && v)
        native::set_ped_can_switch_weapon(native::get_player_ped(-1), true);
}

vehicle_weapons_menu* vehicle_weapons_menu::get() {
    static vehicle_weapons_menu instance;
    return &instance;
}
