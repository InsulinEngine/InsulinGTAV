#include "menu/base/submenus/vehicle_neon.h"
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
    int  g_r = 0, g_g = 100, g_b = 255;
    bool g_on = false;

    void apply() {
        Vehicle v = my_vehicle();
        if (!v) return;
        for (int i = 0; i < 4; i++)
            native::set_vehicle_neon_enabled(v, i, g_on);
        native::set_vehicle_neon_colour(v, g_r, g_g, g_b);
    }
}

void vehicle_neon_menu::load() {
    set_name("Neon");
    set_parent<vehicle_customs_menu>();

    add_option(toggle_option("Neon Lights")
        .add_toggle(g_on)
        .add_click([] { apply(); })
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<int>(SCROLLSELECT, "Red")
        .add_number(g_r, "%i", 15).add_min(0).add_max(255)
        .add_update([](number_option<int>*, int) { apply(); }));
    add_option(number_option<int>(SCROLLSELECT, "Green")
        .add_number(g_g, "%i", 15).add_min(0).add_max(255)
        .add_update([](number_option<int>*, int) { apply(); }));
    add_option(number_option<int>(SCROLLSELECT, "Blue")
        .add_number(g_b, "%i", 15).add_min(0).add_max(255)
        .add_update([](number_option<int>*, int) { apply(); }));
}

vehicle_neon_menu* vehicle_neon_menu::get() {
    static vehicle_neon_menu instance;
    return &instance;
}
