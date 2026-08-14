#include "menu/base/submenus/world_trains.h"
#include "menu/base/submenus/world.h"
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
    bool g_no_trains = false;
}

void world_trains_menu::load() {
    set_name("Train");
    set_parent<world_menu>();

    add_option(toggle_option("Disable Trains")
        .add_toggle(g_no_trains)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Delete All Trains")
        .add_click([] {
            native::delete_all_trains();
            menu::notify::stacked("World", "Trains removed");
        }));
}

void world_trains_menu::feature_update() {
    // SET_RANDOM_TRAINS is a switch, not a per-frame push, but re-asserting it is
    // harmless and survives the game turning them back on after a load.
    if (g_no_trains)
        native::set_random_trains(false);
}

world_trains_menu* world_trains_menu::get() {
    static world_trains_menu instance;
    return &instance;
}
