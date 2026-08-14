#include "menu/base/submenus/world_local_entities.h"
#include "menu/base/submenus/world.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/control.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    bool g_no_peds = false;
    bool g_no_traffic = false;
}

void world_local_entities_menu::load() {
    set_name("Local Entities");
    set_parent<world_menu>();

    add_option(toggle_option("No Pedestrians")
        .add_toggle(g_no_peds).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("No Traffic")
        .add_toggle(g_no_traffic).add_savable(get_submenu_name_stack()));
}

void world_local_entities_menu::feature_update() {
    // Density multipliers are per-frame by design.
    if (g_no_peds)
        native::set_ped_density_multiplier_this_frame(0.f);

    if (g_no_traffic) {
        native::set_vehicle_density_multiplier_this_frame(0.f);
        native::set_random_vehicle_density_multiplier_this_frame(0.f);
        native::set_parked_vehicle_density_multiplier_this_frame(0.f);
    }
}

world_local_entities_menu* world_local_entities_menu::get() {
    static world_local_entities_menu instance;
    return &instance;
}
