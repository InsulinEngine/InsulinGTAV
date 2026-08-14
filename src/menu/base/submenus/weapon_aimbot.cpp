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

namespace {
    bool g_aimbot = false;
    bool g_aiming_required = true;

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
    if (native::get_entity_player_is_free_aiming_at(native::player_id(), &target) && target)
        native::set_ped_shoots_at_coord(ped, 0.f, 0.f, 0.f, false);
}

weapon_aimbot_menu* weapon_aimbot_menu::get() {
    static weapon_aimbot_menu instance;
    return &instance;
}
