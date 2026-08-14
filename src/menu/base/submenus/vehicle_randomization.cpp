#include "menu/base/submenus/vehicle_randomization.h"
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

    bool g_rainbow_primary = false;
    bool g_rainbow_secondary = false;
    int  g_tick = 0;
}

void vehicle_randomization_menu::load() {
    set_name("Randomization");
    set_parent<vehicle_menu>();

    add_option(toggle_option("Rainbow Primary Paint")
        .add_toggle(g_rainbow_primary)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Rainbow Secondary Paint")
        .add_toggle(g_rainbow_secondary)
        .add_savable(get_submenu_name_stack()));
}

void vehicle_randomization_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (!v || (!g_rainbow_primary && !g_rainbow_secondary))
        return;

    // Stepping once every few frames: cycling the colour every frame just looks
    // like flicker rather than a colour change.
    if ((++g_tick % 6) != 0)
        return;

    int c = (g_tick / 6) % 160;
    int p = 0, s = 0;
    native::get_vehicle_colours(v, &p, &s);
    if (g_rainbow_primary)   p = c;
    if (g_rainbow_secondary) s = c;
    native::set_vehicle_colours(v, p, s);
}

vehicle_randomization_menu* vehicle_randomization_menu::get() {
    static vehicle_randomization_menu instance;
    return &instance;
}
