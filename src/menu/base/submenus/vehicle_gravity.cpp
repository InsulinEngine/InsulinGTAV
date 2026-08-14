#include "menu/base/submenus/vehicle_gravity.h"
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

    bool g_slippy = false;
    bool g_freeze = false;
    bool g_autoflip = false;
    bool g_freeze_latched = false;
}

void vehicle_gravity_menu::load() {
    set_name("Gravity");
    set_parent<vehicle_menu>();

    add_option(toggle_option("Drive on Water")
        .add_toggle(g_slippy)
        .add_tooltip("Reduced grip, which is what Ozark drives across water with")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Freeze")
        .add_toggle(g_freeze)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Auto Flip")
        .add_toggle(g_autoflip)
        .add_tooltip("Rights the car when it ends up on its roof")
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Place on Ground")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_on_ground_properly(v, 0); }));
}

void vehicle_gravity_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (!v)
        return;

    if (g_slippy)
        native::set_vehicle_reduce_grip(v, true);

    // Freeze is the one that needs an explicit release.
    if (g_freeze) {
        native::freeze_entity_position(v, true);
        g_freeze_latched = true;
    } else if (g_freeze_latched) {
        native::freeze_entity_position(v, false);
        g_freeze_latched = false;
    }

    if (g_autoflip && native::is_entity_upsidedown(v))
        native::set_vehicle_on_ground_properly(v, 0);
}

vehicle_gravity_menu* vehicle_gravity_menu::get() {
    static vehicle_gravity_menu instance;
    return &instance;
}
