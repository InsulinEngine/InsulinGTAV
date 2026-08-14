#include "menu/base/submenus/teleport_directional.h"
#include "menu/base/submenus/teleport.h"
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
    // Move the vehicle when seated: pulling the ped out from under a car leaves
    // the car behind and drops you through the world.
    Entity subject() {
        Ped ped = native::get_player_ped(-1);
        if (ped && native::is_ped_in_any_vehicle(ped, false)) {
            Vehicle v = native::get_vehicle_ped_is_in(ped, false);
            if (v) return v;
        }
        return ped;
    }

    void warp(float x, float y, float z) {
        Entity e = subject();
        if (e) native::set_entity_coords_no_offset(e, x, y, z, false, false, false);
    }
    float g_distance = 20.f;

    void nudge(float fx, float fy, float fz) {
        Entity e = subject();
        if (!e) return;
        math::vector3<float> c = native::get_entity_coords(e, true);
        math::vector3<float> f = native::get_entity_forward_vector(e);
        // Right is the forward vector turned 90 degrees on z.
        warp(c.x + (f.x * fy + f.y * fx) * g_distance,
             c.y + (f.y * fy - f.x * fx) * g_distance,
             c.z + fz * g_distance);
    }
}

void teleport_directional_menu::load() {
    set_name("Directional");
    set_parent<teleport_menu>();

    add_option(number_option<float>(SCROLLSELECT, "Distance")
        .add_number(g_distance, "%.0f", 5.f).add_min(5.f).add_max(200.f)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Forwards").add_click([] { nudge(0.f,  1.f, 0.f); }));
    add_option(button_option("Backwards").add_click([] { nudge(0.f, -1.f, 0.f); }));
    add_option(button_option("Above").add_click([] { nudge(0.f, 0.f,  1.f); }));
    add_option(button_option("Below").add_click([] { nudge(0.f, 0.f, -1.f); }));
    add_option(button_option("Left").add_click([] { nudge(-1.f, 0.f, 0.f); }));
    add_option(button_option("Right").add_click([] { nudge( 1.f, 0.f, 0.f); }));
}

teleport_directional_menu* teleport_directional_menu::get() {
    static teleport_directional_menu instance;
    return &instance;
}
