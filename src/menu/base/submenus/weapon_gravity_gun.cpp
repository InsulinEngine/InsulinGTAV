#include "menu/base/submenus/weapon_gravity_gun.h"
#include "menu/base/submenus/weapon.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"
#include "game/aim_ray.h"

namespace {
    bool   g_on = false;
    Entity g_held = 0;
    float  g_distance = 15.f;
}

void weapon_gravity_gun_menu::load() {
    set_name("Gravity Gun");
    set_parent<weapon_menu>();

    add_option(toggle_option("Toggle Gravity Gun")
        .add_toggle(g_on)
        .add_tooltip("Shoot something to pick it up, shoot again to drop it")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Hold Distance")
        .add_number(g_distance, "%.0f", 2.f).add_min(3.f).add_max(50.f)
        .add_savable(get_submenu_name_stack()));
}

void weapon_gravity_gun_menu::feature_update() {
    if (!g_on) {
        g_held = 0;
        return;
    }

    // Grab and release on the same trigger, so one toggle covers both.
    if (game::aim::just_fired()) {
        if (g_held) {
            g_held = 0;
        } else {
            game::aim::hit h = game::aim::trace();
            if (h.valid && h.entity)
                g_held = h.entity;
        }
    }

    if (!g_held || !native::does_entity_exist(g_held)) {
        g_held = 0;
        return;
    }

    // Carry it in front of the camera. Setting the position rather than applying
    // force keeps it from oscillating around the target point.
    math::vector3<float> from = native::get_gameplay_cam_coord();
    math::vector3<float> rot  = native::get_gameplay_cam_rot(2);
    const float deg = 0.0174532924f;
    float cp = native::cos(rot.x * deg);
    float x = from.x - native::sin(rot.z * deg) * cp * g_distance;
    float y = from.y + native::cos(rot.z * deg) * cp * g_distance;
    float z = from.z + native::sin(rot.x * deg) * g_distance;
    native::set_entity_coords_no_offset(g_held, x, y, z, false, false, false);
}

weapon_gravity_gun_menu* weapon_gravity_gun_menu::get() {
    static weapon_gravity_gun_menu instance;
    return &instance;
}
