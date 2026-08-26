#include "menu/base/submenus/spawner.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/spawner_peds.h"
#include "menu/base/options/submenu_option.h"

// Top level, as in Ozark. Ozark puts its ped lists directly here. The vehicle
// spawner now lives under the Vehicle menu (vehicle.cpp); this keeps the ped
// list, which is still worth its own submenu rather than burying the menu root.
void spawner_menu::load() {
    set_name("Spawner");
    set_parent<main_menu>();

    add_option(submenu_option("Peds").add_submenu<spawner_peds_menu>());
}

spawner_menu* spawner_menu::get() {
    static spawner_menu instance;
    return &instance;
}
