#include "menu/base/submenus/weapon_aimbot.h"
#include "menu/base/submenus/weapon.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"
#include "rage/invoker/missing_natives.h"

namespace {
    bool g_aimbot = false;
    bool  g_aiming_required = true;
    float g_aim_height = 0.5f;   // metres above the entity origin; tune on the pad

    Ped self_ped() { return native::get_player_ped(-1); }
}

void weapon_aimbot_menu::load() {
    set_name("Aim Assist");
    set_parent<weapon_menu>();

    add_option(toggle_option("Toggle Aimbot")
        .add_toggle(g_aimbot)
        .add_tooltip("Pulls your aim onto the nearest ped in front of you")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Aiming Required")
        .add_toggle(g_aiming_required)
        .add_tooltip("Only assist while you actually hold aim")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Aim Height")
        .add_number(g_aim_height, "%.2f m", 0.1f).add_min(-1.f).add_max(2.f)
        .add_tooltip("Height above the target's origin to shoot at; raise it if shots go low")
        .add_savable(get_submenu_name_stack()));
}

void weapon_aimbot_menu::feature_update() {
    if (!g_aimbot)
        return;

    Ped ped = self_ped();
    if (!ped)
        return;

    // Gate on aiming by default. An aimbot that steers while you walk around is
    // both obvious and unusable, which is why Ozark has the same switch.
    if (g_aiming_required && !native::is_player_free_aiming(native::player_id()))
        return;

    Ped target = 0;
    if (!native::get_entity_player_is_free_aiming_at(native::player_id(), &target) || !target)
        return;

    // Only while the trigger is actually down. Without this the assist fires for
    // you the moment a ped crosses your sights, which is a different feature and
    // not the one on the tin.
    if (!native::is_ped_shooting(ped))
        return;

    // get_entity_coords goes through the verified RVA wrapper, not the hash path -
    // its Vector3 return is the one already proven on console by the player panel.
    // get_ped_bone_coords would be the more precise source, but it exists only as a
    // hash native and that struct-return path has never been checked.
    //
    // The entity origin sits low on a ped, so the shot needs lifting. The exact
    // offset is a guess dressed as a constant, which is why it is a slider: find
    // the number on the pad instead of through a flash cycle.
    math::vector3<float> at = native::get_entity_coords(target, true);
    native::set_ped_shoots_at_coord(ped, at.x, at.y, at.z + g_aim_height, true);
}

weapon_aimbot_menu* weapon_aimbot_menu::get() {
    static weapon_aimbot_menu instance;
    return &instance;
}
