#include "menu/base/submenus/vehicle.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/vehicle_customs.h"
#include "menu/base/submenus/vehicle_health.h"
#include "menu/base/submenus/vehicle_weapons.h"
#include "menu/base/submenus/vehicle_particles.h"
#include "menu/base/submenus/vehicle_movement.h"
#include "menu/base/submenus/vehicle_boost.h"
#include "menu/base/submenus/vehicle_collision.h"
#include "menu/base/submenus/vehicle_gravity.h"
#include "menu/base/submenus/vehicle_multipliers.h"
#include "menu/base/submenus/vehicle_modifiers.h"
#include "menu/base/submenus/vehicle_autopilot.h"
#include "menu/base/submenus/vehicle_ramps.h"
#include "menu/base/submenus/vehicle_randomization.h"
#include "menu/base/submenus/vehicle_seats.h"
#include "menu/base/submenus/vehicle_speedometer.h"
#include "menu/base/submenus/vehicle_doors.h"
#include "menu/base/submenus/vehicle_tyre_tracks.h"
#include "menu/base/submenus/handling_editor.h"
#include "menu/base/submenus/vehicle_spawner.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

// Ozark's Vehicle menu, option for option and in its order, with the handling
// editor added at the end - that one has no Ozark counterpart.

namespace {
    bool g_godmode = false;
    bool g_invisible = false;
    bool g_seatbelt = false;
    bool g_burn_shell = false;

    bool g_invisible_latched = false;

    int g_upgrades = 0;
    int g_downgrades = 0;
    scroll_struct<int> g_upgrade_list[2];
    scroll_struct<int> g_downgrade_list[2];

    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }

    void set_all_mods(bool best) {
        Vehicle v = my_vehicle();
        if (!v)
            return;
        native::set_vehicle_mod_kit(v, 0);
        for (int slot = 0; slot <= 16; slot++) {
            int n = native::get_num_vehicle_mods(v, slot);
            if (n > 0)
                native::set_vehicle_mod(v, slot, best ? n - 1 : 0, false);
        }
        menu::notify::stacked("Vehicle", best ? "Upgraded" : "Downgraded");
    }
}

void vehicle_menu::load() {
    set_name("Vehicle");
    set_parent<main_menu>();

    add_option(toggle_option("Godmode")
        .add_toggle(g_godmode)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Invisibility")
        .add_toggle(g_invisible)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Seatbelt")
        .add_toggle(g_seatbelt)
        .add_tooltip("Stops you being thrown through the windscreen")
        .add_savable(get_submenu_name_stack()));

    static const char* const k_scope[] = { "Performance", "Everything" };
    for (int i = 0; i < 2; i++) {
        g_upgrade_list[i].m_name.set(k_scope[i]);
        g_upgrade_list[i].m_result = i;
        g_downgrade_list[i].m_name.set(k_scope[i]);
        g_downgrade_list[i].m_result = i;
    }

    add_option(scroll_option<int>(SCROLLSELECT, "Upgrades")
        .add_scroll(g_upgrades, 0, 1, g_upgrade_list)
        .add_click([] { set_all_mods(true); }));

    add_option(scroll_option<int>(SCROLLSELECT, "Downgrades")
        .add_scroll(g_downgrades, 0, 1, g_downgrade_list)
        .add_click([] { set_all_mods(false); }));

    add_option(submenu_option("Spawn Vehicle").add_submenu<vehicle_spawner_menu>()
        .add_tooltip("Browse and spawn any vehicle by class"));

    add_option(submenu_option("Customs").add_submenu<vehicle_customs_menu>());
    add_option(submenu_option("Health").add_submenu<vehicle_health_menu>());
    add_option(submenu_option("Weapons").add_submenu<vehicle_weapons_menu>());
    add_option(submenu_option("Particle FX").add_submenu<vehicle_particles_menu>());
    add_option(submenu_option("Movement").add_submenu<vehicle_movement_menu>());
    add_option(submenu_option("Boost").add_submenu<vehicle_boost_menu>());
    add_option(submenu_option("Collision").add_submenu<vehicle_collision_menu>());
    add_option(submenu_option("Gravity").add_submenu<vehicle_gravity_menu>());
    add_option(submenu_option("Multipliers").add_submenu<vehicle_multipliers_menu>());
    add_option(submenu_option("Modifiers").add_submenu<vehicle_modifiers_menu>());
    add_option(submenu_option("Autopilot").add_submenu<vehicle_autopilot_menu>());
    add_option(submenu_option("Ramps").add_submenu<vehicle_ramps_menu>());
    add_option(submenu_option("Randomization").add_submenu<vehicle_randomization_menu>());
    add_option(submenu_option("Seats").add_submenu<vehicle_seats_menu>());
    add_option(submenu_option("Speedometer").add_submenu<vehicle_speedometer_menu>());
    add_option(submenu_option("Doors").add_submenu<vehicle_doors_menu>());
    add_option(submenu_option("Tire Tracks").add_submenu<vehicle_tyre_tracks_menu>());

    add_option(button_option("Clone")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            uint32_t model = native::get_entity_model(v);
            math::vector3<float> c = native::get_entity_coords(v, true);
            float h = native::get_entity_heading(v);
            Vehicle n = native::create_vehicle(model, c.x, c.y + 5.f, c.z, h, false, false, 0);
            native::set_vehicle_on_ground_properly(n, 0);
            native::set_entity_as_mission_entity(n, true, true);
            menu::notify::stacked("Vehicle", n ? "Cloned" : "Clone failed");
        }));

    add_option(toggle_option("Burn Shell")
        .add_toggle(g_burn_shell)
        .add_tooltip("Keeps the car driveable while it burns")
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Delete")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::set_entity_as_mission_entity(v, true, true);
            native::delete_entity(&v);
            menu::notify::stacked("Vehicle", "Deleted");
        }));

    add_option(break_option("Insulin").ref());

    add_option(submenu_option("Handling Editor").add_submenu<handling_editor_menu>()
        .add_tooltip("Live CHandlingData editing - no Ozark counterpart"));
}

void vehicle_menu::feature_update() {
    Vehicle v = my_vehicle();
    if (!v)
        return;

    if (g_godmode)
        native::set_entity_invincible(v, true);

    // Invisibility needs the explicit hand-back; the rest are per-frame flags.
    if (g_invisible) {
        native::set_entity_visible(v, false, false);
        g_invisible_latched = true;
    } else if (g_invisible_latched) {
        native::set_entity_visible(v, true, false);
        g_invisible_latched = false;
    }

    if (g_seatbelt)
        native::set_ped_config_flag(native::get_player_ped(-1), 32, true);

    if (g_burn_shell)
        native::set_vehicle_engine_health(v, 1000.f);
}

vehicle_menu* vehicle_menu::get() {
    static vehicle_menu instance;
    return &instance;
}
