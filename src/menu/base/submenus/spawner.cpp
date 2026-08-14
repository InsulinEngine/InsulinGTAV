#include "menu/base/submenus/spawner.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/vehicle_spawner.h"
#include "menu/base/submenus/spawner_peds.h"
#include "menu/base/options/submenu_option.h"

// Top level, as in Ozark. Ozark puts its ped lists directly here; this splits
// vehicles and peds into their own submenus because the vehicle list is 872 long
// and would bury everything else.
void spawner_menu::load() {
    set_name("Spawner");
    set_parent<main_menu>();

    add_option(submenu_option("Vehicles").add_submenu<vehicle_spawner_menu>()
        .add_tooltip("Every vehicle this build can spawn, by class"));
    add_option(submenu_option("Peds").add_submenu<spawner_peds_menu>());
}

spawner_menu* spawner_menu::get() {
    static spawner_menu instance;
    return &instance;
}
