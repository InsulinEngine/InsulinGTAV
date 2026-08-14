#include "menu/base/submenus/vehicle_health.h"
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

    bool g_auto_repair = false;
    bool g_auto_wash = false;
}

void vehicle_health_menu::load() {
    set_name("Health");
    set_parent<vehicle_menu>();

    add_option(button_option("Repair Vehicle")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::set_vehicle_fixed(v);
            native::set_vehicle_deformation_fixed(v);
            menu::notify::stacked("Vehicle", "Repaired");
        }));

    add_option(button_option("Wash Vehicle")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_dirt_level(v, 0.f); }));

    add_option(button_option("Dirty Vehicle")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_dirt_level(v, 15.f); }));

    add_option(toggle_option("Auto Repair")
        .add_toggle(g_auto_repair)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Auto Wash")
        .add_toggle(g_auto_wash)
        .add_savable(get_submenu_name_stack()));
}

void vehicle_health_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (!v)
        return;

    // Only when there is actually damage: SET_VEHICLE_FIXED every frame kills the
    // engine sound and makes the car sit strangely.
    if (g_auto_repair && native::get_vehicle_engine_health(v) < 1000.f) {
        native::set_vehicle_fixed(v);
        native::set_vehicle_deformation_fixed(v);
    }

    if (g_auto_wash && native::get_vehicle_dirt_level(v) > 0.f)
        native::set_vehicle_dirt_level(v, 0.f);
}

vehicle_health_menu* vehicle_health_menu::get() {
    static vehicle_health_menu instance;
    return &instance;
}
