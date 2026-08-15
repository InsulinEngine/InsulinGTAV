#include "menu/base/submenus/player.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/player_appearance.h"
#include "menu/base/submenus/player_movement.h"
#include "menu/base/submenus/player_animation.h"
#include "menu/base/submenus/player_particles.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "game/camera_dir.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

// Ozark's Player menu, option for option and in its order. Five entries here are
// session-only in Ozark and depend on its player manager and network hooks, which
// this port does not have: Off the Radar, Reveal Hidden Players, Bullshark
// Testosterone, Merryweather Request, Badsport and Kill Killers. They are present
// under their own names and say so when used, rather than being silently dropped
// or pretending to work.

namespace {
    bool g_godmode = false;
    bool g_disable_police = false;
    bool g_disable_ragdoll = false;
    bool g_off_radar = false;
    bool g_blind_eye = false;
    bool g_reveal_hidden = false;
    bool g_peds_ignore = false;
    bool g_reduced_collision = false;
    bool  g_superman = false;
    float g_fly_speed = 30.f;   // m/s; ~4x sprint, well under a helicopter
    bool g_badsport = false;
    bool g_breathe_fire = false;
    bool g_swim_anywhere = false;
    bool g_kill_killers = false;

    // Latches, so each persistent flag is handed back exactly once when its toggle
    // goes off rather than being fought over every frame.
    bool g_police_latched = false;
    bool g_peds_latched = false;
    bool g_ragdoll_latched = false;

    int g_merryweather = 0;
    scroll_struct<int> g_merryweather_list[5];

    Ped self_ped() { return native::get_player_ped(-1); }
    Player self_player() { return native::player_id(); }

    void session_only(const char* what) {
        menu::notify::stacked("Player", what);
    }
}

void player_menu::load() {
    set_name("Player");
    set_parent<main_menu>();

    add_option(submenu_option("Appearance").add_submenu<player_appearance_menu>());
    add_option(submenu_option("Movement").add_submenu<player_movement_menu>());
    add_option(submenu_option("Animation").add_submenu<player_animation_menu>());
    add_option(submenu_option("Particle FX").add_submenu<player_particles_menu>());

    add_option(toggle_option("Godmode")
        .add_toggle(g_godmode)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Police")
        .add_toggle(g_disable_police)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Ragdoll")
        .add_toggle(g_disable_ragdoll)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Off the Radar")
        .add_toggle(g_off_radar)
        .add_tooltip("Session only - needs the network layer this port does not have")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Cops Turn Blind Eye")
        .add_toggle(g_blind_eye)
        .add_tooltip("Cops stop reacting but the stars still rise")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Reveal Hidden Players")
        .add_toggle(g_reveal_hidden)
        .add_tooltip("Session only - needs the player manager this port does not have")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Peds Ignore")
        .add_toggle(g_peds_ignore)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Bullshark Testosterone")
        .add_tooltip("Session only")
        .add_click([] { session_only("Bullshark is session only"); }));

    for (int i = 0; i < 5; i++) {
        static const char* const names[] = { "Pickup", "Airstrike", "Helicopter", "Backup", "Ammo Drop" };
        g_merryweather_list[i].m_name.set(names[i]);
        g_merryweather_list[i].m_result = i;
    }
    add_option(scroll_option<int>(SCROLLSELECT, "Merryweather Request")
        .add_scroll(g_merryweather, 0, 4, g_merryweather_list)
        .add_tooltip("Session only")
        .add_click([] { session_only("Merryweather is session only"); }));

    add_option(button_option("Suicide")
        .add_click([] { native::apply_damage_to_ped(self_ped(), 10000, true); }));

    add_option(button_option("Clone")
        .add_click([] {
            Ped clone = native::clone_ped(self_ped(), false, false, true);
            menu::notify::stacked("Player", clone ? "Cloned" : "Clone failed");
        }));

    add_option(button_option("Health & Armor Regeneration")
        .add_click([] {
            Ped ped = self_ped();
            native::set_entity_health(ped, native::get_ped_max_health(ped));
            native::set_ped_armour(ped, 100);
            menu::notify::stacked("Player", "Restored");
        }));

    add_option(button_option("Sky Dive")
        .add_click([] {
            Ped ped = self_ped();
            math::vector3<float> c = native::get_entity_coords(ped, true);
            native::set_entity_coords_no_offset(ped, c.x, c.y, c.z + 300.f, false, false, false);
            native::task_parachute(ped, true, 0);
        }));

    add_option(toggle_option("Reduced Collision")
        .add_toggle(g_reduced_collision)
        .add_tooltip("Stops crashes knocking you around")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Superman")
        .add_toggle(g_superman)
        .add_tooltip("Hover in place; hold Cross to fly where the camera looks")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Flight Speed")
        .add_number(g_fly_speed, "%.0f m/s", 5.f).add_min(5.f).add_max(150.f)
        .add_tooltip("How fast Superman flies. Sprinting on foot is about 7 m/s")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Badsport")
        .add_toggle(g_badsport)
        .add_tooltip("Session only")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Breathe Fire")
        .add_toggle(g_breathe_fire)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Swim Anywhere")
        .add_toggle(g_swim_anywhere)
        .add_tooltip("No drowning, no air limit")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Kill Killers")
        .add_toggle(g_kill_killers)
        .add_tooltip("Session only - needs the network event hook this port does not have")
        .add_savable(get_submenu_name_stack()));
}

void player_menu::feature_update() {
    Ped ped = self_ped();
    Player player = self_player();
    if (!ped)
        return;

    if (g_godmode)
        native::set_entity_invincible(ped, true);

    if (g_disable_police) {
        native::set_police_ignore_player(player, true);
        g_police_latched = true;
    } else if (g_police_latched) {
        native::set_police_ignore_player(player, false);
        g_police_latched = false;
    }

    if (g_peds_ignore) {
        native::set_everyone_ignore_player(player, true);
        g_peds_latched = true;
    } else if (g_peds_latched) {
        native::set_everyone_ignore_player(player, false);
        g_peds_latched = false;
    }

    if (g_disable_ragdoll) {
        native::set_ped_can_ragdoll(ped, false);
        g_ragdoll_latched = true;
    } else if (g_ragdoll_latched) {
        native::set_ped_can_ragdoll(ped, true);
        g_ragdoll_latched = false;
    }

    // Cops keep escalating the wanted level but stop acting on it, which is what
    // separates this from Disable Police above.
    if (g_blind_eye)
        native::set_player_wanted_level_no_drop(player, 0, false);

    if (g_reduced_collision)
        native::set_ped_can_be_knocked_off_vehicle(ped, 1);

    if (g_swim_anywhere) {
        native::set_ped_max_time_underwater(ped, 1000.f);
        native::set_entity_proofs(ped, false, false, false, false, false, false, false, true);
    }

    if (g_breathe_fire)
        native::set_fire_ammo_this_frame(player);

    // Superman: hover by default, fly along the camera while Cross is held.
    // Setting the velocity every frame is what keeps gravity from taking over -
    // the zero case is the hover, not a no-op.
    //
    // While the menu is open the base disables gameplay controls, so
    // is_control_pressed reads false and you hover instead of flying off while
    // navigating. That is deliberate.
    if (g_superman && !native::is_ped_in_any_vehicle(ped, false)) {
        native::set_ped_can_ragdoll(ped, false);

        if (native::is_control_pressed(0, ControlSprint)) {
            math::vector3<float> d = game::camera_direction();
            native::set_entity_velocity(ped, d.x * g_fly_speed,
                                             d.y * g_fly_speed,
                                             d.z * g_fly_speed);
        } else {
            native::set_entity_velocity(ped, 0.f, 0.f, 0.f);
        }
    }
}

player_menu* player_menu::get() {
    static player_menu instance;
    return &instance;
}
