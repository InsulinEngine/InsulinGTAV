#include "menu/base/submenus/world_bullet_tracers.h"
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
    bool g_tracers = false;
    int  g_r = 255, g_g = 0, g_b = 0;
}

void world_bullet_tracers_menu::load() {
    set_name("Bullet Tracers");
    set_parent<world_menu>();

    add_option(toggle_option("Draw Tracers")
        .add_toggle(g_tracers)
        .add_tooltip("Draws a line along your shots")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<int>(SCROLLSELECT, "Red")
        .add_number(g_r, "%i", 15).add_min(0).add_max(255)
        .add_savable(get_submenu_name_stack()));
    add_option(number_option<int>(SCROLLSELECT, "Green")
        .add_number(g_g, "%i", 15).add_min(0).add_max(255)
        .add_savable(get_submenu_name_stack()));
    add_option(number_option<int>(SCROLLSELECT, "Blue")
        .add_number(g_b, "%i", 15).add_min(0).add_max(255)
        .add_savable(get_submenu_name_stack()));
}

void world_bullet_tracers_menu::feature_update() {
    if (!g_tracers)
        return;

    Ped ped = native::get_player_ped(-1);
    if (!ped || !native::is_ped_shooting(ped))
        return;

    // Muzzle to impact. GET_PED_LAST_WEAPON_IMPACT_COORD only returns something on
    // the frames a shot actually lands, which is what keeps this from drawing a
    // stale line for the whole burst.
    math::vector3<float> impact = { 0.f, 0.f, 0.f };
    if (!native::get_ped_last_weapon_impact_coord(ped, &impact))
        return;

    math::vector3<float> from = native::get_ped_bone_coords(ped, 6286, 0.f, 0.f, 0.f);
    native::draw_line(from.x, from.y, from.z, impact.x, impact.y, impact.z, g_r, g_g, g_b, 200);
}

world_bullet_tracers_menu* world_bullet_tracers_menu::get() {
    static world_bullet_tracers_menu instance;
    return &instance;
}
