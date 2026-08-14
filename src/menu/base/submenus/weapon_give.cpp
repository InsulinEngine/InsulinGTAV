#include "menu/base/submenus/weapon_give.h"
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
    const char* const k_weapons[] = {
        "WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50",
        "WEAPON_MICROSMG", "WEAPON_SMG", "WEAPON_ASSAULTSMG", "WEAPON_CARBINERIFLE",
        "WEAPON_ASSAULTRIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG", "WEAPON_COMBATMG",
        "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN",
        "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_RPG", "WEAPON_GRENADELAUNCHER",
        "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_MOLOTOV",
        "WEAPON_KNIFE", "WEAPON_BAT", "WEAPON_CROWBAR", "WEAPON_HAMMER",
    };
    constexpr int WEAPON_COUNT = (int)(sizeof(k_weapons) / sizeof(k_weapons[0]));

    Ped self_ped() { return native::get_player_ped(-1); }
}

void weapon_give_menu::load() {
    set_name("Give Weapons and Ammo");
    set_parent<weapon_menu>();

    add_option(button_option("Give All Weapons")
        .add_click([] {
            Ped ped = self_ped();
            for (int i = 0; i < WEAPON_COUNT; i++)
                native::give_weapon_to_ped(ped, native::get_hash_key(k_weapons[i]), 9999, false, false);
            menu::notify::stacked("Weapon", "Weapons given");
        }));

    add_option(button_option("Max Weapon Ammo")
        .add_click([] {
            Ped ped = self_ped();
            for (int i = 0; i < WEAPON_COUNT; i++)
                native::set_ped_ammo(ped, native::get_hash_key(k_weapons[i]), 9999, 0);
            menu::notify::stacked("Weapon", "Ammo maxed");
        }));

    add_option(button_option("Give Current Weapon Ammo")
        .add_click([] {
            Ped ped = self_ped();
            uint32_t w = 0;
            native::get_current_ped_weapon(ped, &w, true);
            if (w) native::set_ped_ammo(ped, w, 9999, 0);
        }));
}

weapon_give_menu* weapon_give_menu::get() {
    static weapon_give_menu instance;
    return &instance;
}
