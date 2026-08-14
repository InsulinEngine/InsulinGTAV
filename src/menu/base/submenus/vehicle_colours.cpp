#include "menu/base/submenus/vehicle_colours.h"
#include "menu/base/submenus/vehicle_customs.h"
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
    int g_primary = 0;
    int g_secondary = 0;
    int g_pearl = 0;
    int g_wheel = 0;
}

void vehicle_colours_menu::load() {
    set_name("Colours");
    set_parent<vehicle_customs_menu>();

    add_option(number_option<int>(SCROLLSELECT, "Primary")
        .add_number(g_primary, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_colours(v, g_primary, g_secondary);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Secondary")
        .add_number(g_secondary, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_colours(v, g_primary, g_secondary);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Pearlescent")
        .add_number(g_pearl, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_extra_colours(v, g_pearl, g_wheel);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Wheel Colour")
        .add_number(g_wheel, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_extra_colours(v, g_pearl, g_wheel);
        }));

    add_option(break_option("Windows").ref());

    add_option(button_option("Clear Tint")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_window_tint(v, 0); }));
    add_option(button_option("Limo Tint")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_window_tint(v, 5); }));
}

vehicle_colours_menu* vehicle_colours_menu::get() {
    static vehicle_colours_menu instance;
    return &instance;
}
