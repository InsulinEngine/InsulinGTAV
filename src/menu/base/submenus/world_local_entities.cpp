#include "menu/base/submenus/world_local_entities.h"
#include "menu/base/submenus/world.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/control.h"
#include "menu/base/util/esp.h"
#include "menu/base/submenus/helper_esp.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    bool g_no_peds = false;
    bool g_no_traffic = false;

    // Vehicle ESP only. World ped and object ESP are out of scope here: the
    // port has no ped or object pool enumeration (native::get_all_vehicles
    // exists; get_all_peds/get_all_objects/get_all_pickups do not, and this
    // file keeps no entity list of its own - it only sets density multipliers).
    // Walking the game's ped/object pools directly is separate RE work this
    // task does not do; this is a scoping decision, not an oversight.
    menu::esp::esp_context g_vehicle_esp;

    // GET_ALL_VEHICLES takes no size and writes one handle per pooled
    // vehicle, so the buffer must be sized for the pool rather than for what
    // we intend to read. Static rather than on the stack: 4KB per frame is a
    // lot on a game thread, and a POD array in .bss needs no constructor, so
    // it adds no .init_array entry. 1024 is the size community scripts use
    // for this native and is comfortably above GTA V's vehicle pool ceiling.
    static Any g_vehicle_handles[1024];
}

void world_local_entities_menu::load() {
    set_name("Local Entities");
    set_parent<world_menu>();

    add_option(toggle_option("No Pedestrians")
        .add_toggle(g_no_peds).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("No Traffic")
        .add_toggle(g_no_traffic).add_savable(get_submenu_name_stack()));

    add_option(submenu_option("Vehicle ESP")
        .add_submenu<helper_esp_menu>()
        .add_click([] { helper_esp_menu::open_for(&g_vehicle_esp, "Vehicle ESP"); }));
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

    if (g_vehicle_esp.any()) {
        constexpr int cap = sizeof(g_vehicle_handles) / sizeof(g_vehicle_handles[0]);
        int n = native::get_all_vehicles(g_vehicle_handles);
        if (n > cap) n = cap;
        if (n < 0) n = 0;
        for (int i = 0; i < n; i++)
            menu::esp::draw_entity(g_vehicle_esp, (Entity)g_vehicle_handles[i], nullptr);
    }
}

world_local_entities_menu* world_local_entities_menu::get() {
    static world_local_entities_menu instance;
    return &instance;
}
