#include "menu/base/submenus/vehicle_doors.h"
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

    int g_open = 0;
    int g_close = 0;
    int g_delete = 0;
}

void vehicle_doors_menu::load() {
    set_name("Doors");
    set_parent<vehicle_menu>();

    add_option(number_option<int>(SCROLLSELECT, "Open Door")
        .add_number(g_open, "%i", 1).add_min(0).add_max(5)
        .add_tooltip("0 front left, 1 front right, 2 rear left, 3 rear right, 4 hood, 5 boot")
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_door_open(v, g_open, false, false);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Close Door")
        .add_number(g_close, "%i", 1).add_min(0).add_max(5)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_door_shut(v, g_close, false);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Delete Door")
        .add_number(g_delete, "%i", 1).add_min(0).add_max(5)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_door_broken(v, g_delete, false);
        }));

    add_option(button_option("Lock Doors")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_doors_locked(v, 2); }));

    add_option(button_option("Unlock Doors")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_doors_locked(v, 1); }));
}

void vehicle_doors_menu::feature_update() {
}

vehicle_doors_menu* vehicle_doors_menu::get() {
    static vehicle_doors_menu instance;
    return &instance;
}
