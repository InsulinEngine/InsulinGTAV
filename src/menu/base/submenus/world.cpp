#include "menu/base/submenus/world.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/world_local_entities.h"
#include "menu/base/submenus/world_game_fx.h"
#include "menu/base/submenus/world_weather.h"
#include "menu/base/submenus/world_time.h"
#include "menu/base/submenus/world_clear_area.h"
#include "menu/base/submenus/world_ocean.h"
#include "menu/base/submenus/world_bullet_tracers.h"
#include "menu/base/submenus/world_trains.h"
#include "menu/base/options/button.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"

// Ozark's World menu. Bullet Tracers, World Waypoint and Train are not here yet -
// each needs its own asset or entity handling rather than a native call.
void world_menu::load() {
    set_name("World");
    set_parent<main_menu>();

    add_option(submenu_option("Local Entities").add_submenu<world_local_entities_menu>());
    add_option(submenu_option("Game FX").add_submenu<world_game_fx_menu>());
    add_option(submenu_option("Weather").add_submenu<world_weather_menu>());
    add_option(submenu_option("Time").add_submenu<world_time_menu>());
    add_option(submenu_option("Clear Area").add_submenu<world_clear_area_menu>());
    add_option(submenu_option("Bullet Tracers").add_submenu<world_bullet_tracers_menu>());
    add_option(submenu_option("Ocean").add_submenu<world_ocean_menu>());
    add_option(submenu_option("Train").add_submenu<world_trains_menu>());

    add_option(button_option("Interior Refresh")
        .add_tooltip("Reloads the interior you are standing in")
        .add_click([] {
            Ped ped = native::get_player_ped(-1);
            int interior = native::get_interior_from_entity(ped);
            if (interior) {
                native::refresh_interior(interior);
                menu::notify::stacked("World", "Interior refreshed");
            } else {
                menu::notify::stacked("World", "Not in an interior");
            }
        }));
}

void world_menu::feature_update() {}

world_menu* world_menu::get() {
    static world_menu instance;
    return &instance;
}
