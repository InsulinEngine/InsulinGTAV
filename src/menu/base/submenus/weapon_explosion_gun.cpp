#include "menu/base/submenus/weapon_explosion_gun.h"
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
    bool g_on = false;
    int  g_type = 4;      // 4 = a plain grenade-sized blast
    float g_scale = 1.f;
}

void weapon_explosion_gun_menu::load() {
    set_name("Explosion Gun");
    set_parent<weapon_menu>();

    add_option(toggle_option("Toggle Explosion Gun")
        .add_toggle(g_on).add_savable(get_submenu_name_stack()));

    add_option(number_option<int>(SCROLLSELECT, "Explosion Type")
        .add_number(g_type, "%i", 1).add_min(0).add_max(50)
        .add_tooltip("The game's explosion ids; 4 is a grenade")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Scale")
        .add_number(g_scale, "%.1f", 0.5f).add_min(0.1f).add_max(10.f)
        .add_savable(get_submenu_name_stack()));
}

void weapon_explosion_gun_menu::feature_update() {
    if (!g_on || !game::aim::just_fired())
        return;

    game::aim::hit h = game::aim::trace();
    if (h.valid)
        native::add_explosion(h.pos.x, h.pos.y, h.pos.z, g_type, g_scale, true, false, 1.f, false);
}

weapon_explosion_gun_menu* weapon_explosion_gun_menu::get() {
    static weapon_explosion_gun_menu instance;
    return &instance;
}
