#include "menu/base/submenus/self.h"
#include "menu/base/submenus/main.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"

// Feature state (POD globals; the savable toggles persist via config).
static bool g_godmode = false;
static bool g_never_wanted = false;
static bool g_super_jump = false;

static Ped self_ped() { return native::get_player_ped(-1); }
static Player self_player() { return native::player_id(); }

static void give_all_weapons() {
    Ped ped = self_ped();
    const char* weapons[] = {
        "WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_SMG", "WEAPON_CARBINERIFLE",
        "WEAPON_ASSAULTRIFLE", "WEAPON_PUMPSHOTGUN", "WEAPON_SNIPERRIFLE", "WEAPON_RPG",
        "WEAPON_GRENADE", "WEAPON_KNIFE", "WEAPON_MICROSMG", "WEAPON_MINIGUN",
    };
    for (const char* w : weapons) {
        native::give_weapon_to_ped(ped, native::get_hash_key(w), 9999, false, false);
    }
}

void self_menu::load() {
    set_name("Self");
    set_parent<main_menu>();

    add_option(toggle_option("Godmode")
        .add_toggle(g_godmode)
        .add_click([] { native::set_entity_invincible(self_ped(), g_godmode); })
        .add_savable(get_submenu_name_stack())
        .add_tooltip("Invincibility, re-applied every frame"));

    add_option(toggle_option("Never Wanted")
        .add_toggle(g_never_wanted)
        .add_savable(get_submenu_name_stack())
        .add_tooltip("Keeps the wanted level cleared"));

    add_option(toggle_option("Super Jump")
        .add_toggle(g_super_jump)
        .add_savable(get_submenu_name_stack())
        .add_tooltip("Hold to leap; applied each frame"));

    add_option(break_option("Actions").ref());

    add_option(button_option("Heal")
        .add_click([] { native::set_entity_health(self_ped(), native::get_ped_max_health(self_ped())); }));

    add_option(button_option("Full Armor")
        .add_click([] { native::set_ped_armour(self_ped(), 100); }));

    add_option(button_option("Give All Weapons")
        .add_click([] { give_all_weapons(); menu::notify::stacked("Self", "Weapons given"); }));

    add_option(button_option("Clear Wanted")
        .add_click([] { native::clear_player_wanted_level(self_player()); native::set_player_wanted_level_now(self_player(), false); }));
}

void self_menu::feature_update() {
    if (g_godmode) native::set_entity_invincible(self_ped(), true);
    if (g_super_jump) native::set_super_jump_this_frame(self_player());
    if (g_never_wanted) {
        native::clear_player_wanted_level(self_player());
        native::set_player_wanted_level_now(self_player(), false);
    }
}

self_menu* self_menu::get() {
    static self_menu instance;
    return &instance;
}
