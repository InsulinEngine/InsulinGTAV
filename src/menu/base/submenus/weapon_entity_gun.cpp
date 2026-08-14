#include "menu/base/submenus/weapon_entity_gun.h"
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
    int  g_choice = 0;
    scroll_struct<int> g_list[4];

    // Spawning by hash, so no model list is needed for four fixed entries.
    const char* const k_models[] = { "prop_barrel_01a", "prop_beachball_01",
                                     "prop_bin_01a", "prop_cs_cardbox_01" };
}

void weapon_entity_gun_menu::load() {
    set_name("Entity Gun");
    set_parent<weapon_menu>();

    for (int i = 0; i < 4; i++) {
        g_list[i].m_name.set(k_models[i]);
        g_list[i].m_result = i;
    }

    add_option(toggle_option("Toggle Entity Gun")
        .add_toggle(g_on)
        .add_tooltip("Spawns the chosen object where you shoot")
        .add_savable(get_submenu_name_stack()));

    add_option(scroll_option<int>(SCROLLSELECT, "Object")
        .add_scroll(g_choice, 0, 3, g_list)
        .add_savable(get_submenu_name_stack()));
}

void weapon_entity_gun_menu::feature_update() {
    if (!g_on || !game::aim::just_fired())
        return;

    game::aim::hit h = game::aim::trace();
    if (!h.valid)
        return;

    uint32_t model = native::get_hash_key(k_models[g_choice]);
    if (!native::is_model_in_cdimage(model))
        return;

    // Request and bail if it is not in yet: the next shot will land once the
    // streamer has caught up, which beats blocking the frame on a wait.
    native::request_model(model);
    if (!native::has_model_loaded(model))
        return;

    Object o = native::create_object_no_offset(model, h.pos.x, h.pos.y, h.pos.z + 1.f, true, true, true);
    native::set_entity_as_mission_entity(o, true, true);
    native::set_model_as_no_longer_needed(model);
}

weapon_entity_gun_menu* weapon_entity_gun_menu::get() {
    static weapon_entity_gun_menu instance;
    return &instance;
}
