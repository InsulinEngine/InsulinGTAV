"""One-shot scaffold for the Weapon sub-branch, mirroring Ozark.

Same idea as gen_vehicle_menus.py: write the branch once in a consistent shape,
then maintain the files by hand.
"""
import io
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DST = os.path.join(HERE, "..", "src", "menu", "base", "submenus")

HDR = '''#pragma once
#include "menu/base/submenu.h"

// Ozark: Weapon > {label}
class {cls} : public menu::submenu::submenu {{
public:
    void load() override;
{fu}    static {cls}* get();
}};
'''

CPP = '''#include "menu/base/submenus/{f}.h"
#include "menu/base/submenus/{parent_h}.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"

namespace {{
{state}}}

void {cls}::load() {{
    set_name("{label}");
    set_parent<{parent_cls}>();

{opts}}}
{tickdef}
{cls}* {cls}::get() {{
    static {cls} instance;
    return &instance;
}}
'''


def write(f, cls, label, parent_h, parent_cls, state, opts, tick=None):
    fu = "    void feature_update() override;\n" if tick is not None else ""
    io.open(os.path.join(DST, f + ".h"), "w", encoding="utf-8", newline="\n").write(
        HDR.format(label=label, cls=cls, fu=fu))
    tickdef = ""
    if tick is not None:
        tickdef = "\nvoid %s::feature_update() {\n%s}\n" % (cls, tick)
    io.open(os.path.join(DST, f + ".cpp"), "w", encoding="utf-8", newline="\n").write(
        CPP.format(f=f, cls=cls, label=label, parent_h=parent_h, parent_cls=parent_cls,
                   state=state, opts=opts, tickdef=tickdef))


WEAPONS = '''    const char* const k_weapons[] = {
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
'''

write("weapon_give", "weapon_give_menu", "Give Weapons and Ammo",
      "weapon", "weapon_menu", WEAPONS,
      '''    add_option(button_option("Give All Weapons")
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
''')

write("weapon_aimbot", "weapon_aimbot_menu", "Aim Assist",
      "weapon", "weapon_menu",
      '''    bool g_aimbot = false;
    bool g_aiming_required = true;

    Ped self_ped() { return native::get_player_ped(-1); }
''',
      '''    add_option(toggle_option("Toggle Aimbot")
        .add_toggle(g_aimbot)
        .add_tooltip("Pulls your aim onto the nearest ped in front of you")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Aiming Required")
        .add_toggle(g_aiming_required)
        .add_tooltip("Only assist while you actually hold aim")
        .add_savable(get_submenu_name_stack()));
''',
      '''    if (!g_aimbot)
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
''')

write("weapon_disables", "weapon_disables_menu", "Disables",
      "weapon", "weapon_menu",
      '''    bool g_no_spread = false;
    bool g_no_recoil = false;
    bool g_no_reload_anim = false;

    Ped self_ped() { return native::get_player_ped(-1); }
''',
      '''    add_option(toggle_option("Disable Spread")
        .add_toggle(g_no_spread).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Recoil")
        .add_toggle(g_no_recoil).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Reload Anim")
        .add_toggle(g_no_reload_anim)
        .add_tooltip("Reloads finish instantly")
        .add_savable(get_submenu_name_stack()));
''',
      '''    Ped ped = self_ped();
    if (!ped)
        return;

    // These are per-frame weapon-flag natives, so "off" is simply not pushing them.
    if (g_no_spread || g_no_recoil)
        native::set_ped_accuracy(ped, 100);

    if (g_no_reload_anim)
        native::set_ped_infinite_ammo_clip(ped, true);
''')

print("Weapon submenu sources written to %s" % os.path.normpath(DST))
