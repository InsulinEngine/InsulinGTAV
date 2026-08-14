#include "menu/base/submenus/vehicle_particles.h"
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

    bool g_ptfx = false;
}

void vehicle_particles_menu::load() {
    set_name("Particle FX");
    set_parent<vehicle_menu>();

    add_option(toggle_option("Toggle Particle FX")
        .add_toggle(g_ptfx)
        .add_tooltip("Ozark plays a chosen effect on the car. The picker needs the "
                     "PTFX asset lists, which are not ported yet.")
        .add_savable(get_submenu_name_stack()));
}

void vehicle_particles_menu::feature_update() {
}

vehicle_particles_menu* vehicle_particles_menu::get() {
    static vehicle_particles_menu instance;
    return &instance;
}
