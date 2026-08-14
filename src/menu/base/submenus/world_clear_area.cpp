#include "menu/base/submenus/world_clear_area.h"
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
    float g_radius = 200.f;

    math::vector3<float> here() {
        return native::get_entity_coords(native::get_player_ped(-1), true);
    }
}

void world_clear_area_menu::load() {
    set_name("Clear Area");
    set_parent<world_menu>();

    add_option(number_option<float>(SCROLLSELECT, "Radius")
        .add_number(g_radius, "%.0f", 25.f).add_min(25.f).add_max(1000.f)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Clear Peds")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area_of_peds(c.x, c.y, c.z, g_radius, 0);
            menu::notify::stacked("World", "Peds cleared");
        }));

    add_option(button_option("Clear Vehicles")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area_of_vehicles(c.x, c.y, c.z, g_radius, false, false, false, false, false, 0);
            menu::notify::stacked("World", "Vehicles cleared");
        }));

    add_option(button_option("Clear Objects")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area_of_objects(c.x, c.y, c.z, g_radius, 0);
        }));

    add_option(button_option("Clear Everything")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area(c.x, c.y, c.z, g_radius, true, false, false, false);
            menu::notify::stacked("World", "Area cleared");
        }));
}

world_clear_area_menu* world_clear_area_menu::get() {
    static world_clear_area_menu instance;
    return &instance;
}
