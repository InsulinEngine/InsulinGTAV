#include "menu/base/submenus/vehicle_customs.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/submenus/vehicle_colours.h"
#include "menu/base/submenus/vehicle_neon.h"
#include "menu/base/submenus/vehicle_plate.h"
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
}

void vehicle_customs_menu::load() {
    set_name("Customs");
    set_parent<vehicle_menu>();

    add_option(submenu_option("Colours").add_submenu<vehicle_colours_menu>());
    add_option(submenu_option("Neon").add_submenu<vehicle_neon_menu>());
    add_option(submenu_option("Number Plate").add_submenu<vehicle_plate_menu>());

    add_option(button_option("Max Performance")
        .add_tooltip("Fits the best engine, brakes, transmission, suspension and a turbo")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::set_vehicle_mod_kit(v, 0);
            for (int slot = 11; slot <= 16; slot++) {
                int n = native::get_num_vehicle_mods(v, slot);
                if (n > 0) native::set_vehicle_mod(v, slot, n - 1, false);
            }
            native::toggle_vehicle_mod(v, 18, true);
            menu::notify::stacked("Customs", "Upgraded");
        }));

    add_option(button_option("Remove All Mods")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::set_vehicle_mod_kit(v, 0);
            for (int slot = 0; slot <= 49; slot++)
                native::remove_vehicle_mod(v, slot);
            menu::notify::stacked("Customs", "Stock");
        }));
}

void vehicle_customs_menu::feature_update() {
}

vehicle_customs_menu* vehicle_customs_menu::get() {
    static vehicle_customs_menu instance;
    return &instance;
}
