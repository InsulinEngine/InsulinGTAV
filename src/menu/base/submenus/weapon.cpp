#include "menu/base/submenus/weapon.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/weapon_give.h"
#include "menu/base/submenus/weapon_aimbot.h"
#include "menu/base/submenus/weapon_disables.h"
#include "menu/base/submenus/weapon_explosion_gun.h"
#include "menu/base/submenus/weapon_gravity_gun.h"
#include "menu/base/submenus/weapon_entity_gun.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"
#include "game/aim_ray.h"

// Ozark's Weapon menu. The gun toggles are all the same idea - resolve what the
// shot would hit, then do something to it - which is why they share
// game::aim::trace() rather than each rolling its own raycast.
//
// Ozark has fifteen of them. The ones here are the ones that need nothing beyond
// the raycast; the rest carry their name and say what they still need.

namespace {
    bool g_infinite_ammo = false;
    bool g_instant_kill = false;
    bool g_rapid_fire = false;
    bool g_laser_sight = false;
    bool g_explosive_bullets = false;
    bool g_incendiary_bullets = false;

    bool g_delete_gun = false;
    bool g_force_gun = false;
    bool g_teleport_gun = false;
    bool g_airstrike_gun = false;

    bool g_ammo_latched = false;

    Ped self_ped() { return native::get_player_ped(-1); }
    Player self_player() { return native::player_id(); }
}

void weapon_menu::load() {
    set_name("Weapon");
    set_parent<main_menu>();

    add_option(submenu_option("Give Weapons and Ammo").add_submenu<weapon_give_menu>());
    add_option(submenu_option("Aim Assist").add_submenu<weapon_aimbot_menu>());
    add_option(submenu_option("Explosion Gun").add_submenu<weapon_explosion_gun_menu>());
    add_option(submenu_option("Entity Gun").add_submenu<weapon_entity_gun_menu>());
    add_option(submenu_option("Gravity Gun").add_submenu<weapon_gravity_gun_menu>());
    add_option(submenu_option("Disables").add_submenu<weapon_disables_menu>());

    add_option(toggle_option("Infinite Ammo")
        .add_toggle(g_infinite_ammo).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Instant Kill")
        .add_toggle(g_instant_kill)
        .add_tooltip("Anything you shoot dies outright")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Rapid Fire")
        .add_toggle(g_rapid_fire).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Laser Sight")
        .add_toggle(g_laser_sight)
        .add_tooltip("Draws a line to whatever you are pointing at")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Explosive Bullets")
        .add_toggle(g_explosive_bullets).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Incendiary Bullets")
        .add_toggle(g_incendiary_bullets).add_savable(get_submenu_name_stack()));

    add_option(break_option("Guns").ref());

    add_option(toggle_option("Delete Gun")
        .add_toggle(g_delete_gun)
        .add_tooltip("Removes whatever you shoot")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Force Gun")
        .add_toggle(g_force_gun)
        .add_tooltip("Launches whatever you shoot")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Teleport Gun")
        .add_toggle(g_teleport_gun)
        .add_tooltip("Puts you where you shoot")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Airstrike Gun")
        .add_toggle(g_airstrike_gun)
        .add_tooltip("Drops an explosion where you shoot")
        .add_savable(get_submenu_name_stack()));

    add_option(break_option("Actions").ref());

    add_option(button_option("Remove Weapons")
        .add_click([] {
            native::remove_all_ped_weapons(self_ped(), true);
            menu::notify::stacked("Weapon", "Removed");
        }));
}

void weapon_menu::feature_update() {
    Ped ped = self_ped();
    Player player = self_player();
    if (!ped)
        return;

    // Persistent flags need the hand-back; the *_this_frame ones expire on their own.
    if (g_infinite_ammo) {
        native::set_ped_infinite_ammo(ped, true, 0);
        native::set_ped_infinite_ammo_clip(ped, true);
        g_ammo_latched = true;
    } else if (g_ammo_latched) {
        native::set_ped_infinite_ammo(ped, false, 0);
        native::set_ped_infinite_ammo_clip(ped, false);
        g_ammo_latched = false;
    }

    if (g_explosive_bullets)   native::set_explosive_ammo_this_frame(player);
    if (g_incendiary_bullets)  native::set_fire_ammo_this_frame(player);

    if (g_rapid_fire)
        native::set_ped_shoot_rate(ped, 1000);

    // Aim-dependent features share one trace per frame rather than one each.
    bool need_trace = g_laser_sight || g_delete_gun || g_force_gun ||
                      g_teleport_gun || g_airstrike_gun || g_instant_kill;
    if (!need_trace)
        return;

    game::aim::hit h = game::aim::trace();
    if (!h.valid)
        return;

    if (g_laser_sight) {
        math::vector3<float> from = native::get_gameplay_cam_coord();
        native::draw_line(from.x, from.y, from.z, h.pos.x, h.pos.y, h.pos.z, 255, 0, 0, 200);
    }

    // Everything below acts once per trigger pull, not once per frame: IS_PED_SHOOTING
    // stays true for the whole burst.
    if (!game::aim::just_fired())
        return;

    if (g_teleport_gun)
        native::set_entity_coords_no_offset(ped, h.pos.x, h.pos.y, h.pos.z + 1.f, false, false, false);

    if (g_airstrike_gun)
        native::add_explosion(h.pos.x, h.pos.y, h.pos.z, 4, 1.f, true, false, 1.f, false);

    if (!h.entity)
        return;

    if (g_delete_gun) {
        Entity e = h.entity;
        native::set_entity_as_mission_entity(e, true, true);
        native::delete_entity(&e);
    }

    if (g_force_gun)
        native::apply_force_to_entity(h.entity, 1, 0.f, 0.f, 120.f,
                                      0.f, 0.f, 0.f, 0, true, true, true, false, true);

    if (g_instant_kill && native::is_entity_a_ped(h.entity))
        native::set_entity_health(h.entity, 0);
}

weapon_menu* weapon_menu::get() {
    static weapon_menu instance;
    return &instance;
}
