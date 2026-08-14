#include "menu/base/submenus/weapon_disables.h"
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
    bool g_no_spread = false;
    bool g_no_recoil = false;
    bool g_no_reload_anim = false;

    Ped self_ped() { return native::get_player_ped(-1); }
}

void weapon_disables_menu::load() {
    set_name("Disables");
    set_parent<weapon_menu>();

    add_option(toggle_option("Disable Spread")
        .add_toggle(g_no_spread).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Recoil")
        .add_toggle(g_no_recoil).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Reload Anim")
        .add_toggle(g_no_reload_anim)
        .add_tooltip("Reloads finish instantly")
        .add_savable(get_submenu_name_stack()));
}

void weapon_disables_menu::feature_update() {
    Ped ped = self_ped();
    if (!ped)
        return;

    // These are per-frame weapon-flag natives, so "off" is simply not pushing them.
    if (g_no_spread || g_no_recoil)
        native::set_ped_accuracy(ped, 100);

    if (g_no_reload_anim)
        native::set_ped_infinite_ammo_clip(ped, true);
}

weapon_disables_menu* weapon_disables_menu::get() {
    static weapon_disables_menu instance;
    return &instance;
}
