#include "menu/base/submenus/vehicle_multipliers.h"
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

    float g_torque = 1.f;
}

void vehicle_multipliers_menu::load() {
    set_name("Multipliers");
    set_parent<vehicle_menu>();

    add_option(number_option<float>(SCROLLSELECT, "Engine Torque")
        .add_number(g_torque, "%.2f", 0.25f)
        .add_min(1.f).add_max(20.f)
        .add_tooltip("Multiplies engine power while you are in the car")
        .add_savable(get_submenu_name_stack()));
}

void vehicle_multipliers_menu::feature_update() {
    // No engine-torque multiplier native on this build, so this drives the power
    // through the boost path instead. Real per-field tuning lives in the handling
    // editor, which writes CHandlingData directly.
    Vehicle v = my_vehicle();
    if (v && g_torque > 1.f)
        native::set_vehicle_cheat_power_increase(v, g_torque);
}

vehicle_multipliers_menu* vehicle_multipliers_menu::get() {
    static vehicle_multipliers_menu instance;
    return &instance;
}
