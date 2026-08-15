#include "menu/menu.h"
#include "menu/base/base.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/submenus/main.h"
#include "rage/invoker/hash_natives.h"
#include "menu/base/submenus/player.h"
#include "menu/base/submenus/player_animation.h"
#include "menu/base/submenus/player_animations.h"
#include "menu/base/submenus/player_scenarios.h"
#include "menu/base/submenus/player_clipsets.h"
#include "menu/base/submenus/player_wardrobe_saveload.h"
#include "menu/base/submenus/player_particles.h"
#include "menu/base/submenus/player_particle_manager.h"
#include "menu/base/submenus/player_hand_trails.h"
#include "menu/base/submenus/player_model.h"
#include "menu/base/submenus/player_wardrobe.h"
#include "menu/base/submenus/network.h"
#include "menu/base/submenus/network_players.h"
#include "menu/base/submenus/protections.h"
#include "menu/base/submenus/teleport.h"
#include "menu/base/submenus/teleport_directional.h"
#include "menu/base/submenus/teleport_ipl.h"
#include "menu/base/submenus/teleport_save_load.h"
#include "menu/base/submenus/misc_camera.h"
#include "menu/base/submenus/misc_radio.h"
#include "menu/base/submenus/misc_visions.h"
#include "menu/base/submenus/misc_disables.h"
#include "menu/base/submenus/misc_dispatch.h"
#include "menu/base/submenus/weapon.h"
#include "menu/base/submenus/weapon_explosion_gun.h"
#include "menu/base/submenus/weapon_gravity_gun.h"
#include "menu/base/submenus/weapon_entity_gun.h"
#include "menu/base/submenus/vehicle_colours.h"
#include "menu/base/submenus/vehicle_neon.h"
#include "menu/base/submenus/vehicle_plate.h"
#include "menu/base/submenus/settings_themes.h"
#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenus/helper_color_presets.h"
#include "menu/base/submenus/helper_color_sync.h"
#include "menu/base/submenus/weapon_give.h"
#include "menu/base/submenus/weapon_disables.h"
#include "menu/base/submenus/spawner.h"
#include "menu/base/submenus/world.h"
#include "menu/base/submenus/world_local_entities.h"
#include "menu/base/submenus/world_game_fx.h"
#include "menu/base/submenus/world_weather.h"
#include "menu/base/submenus/world_time.h"
#include "menu/base/submenus/world_clear_area.h"
#include "menu/base/submenus/world_ocean.h"
#include "menu/base/submenus/spawner_peds.h"
#include "menu/base/submenus/misc.h"
#include "menu/base/submenus/misc_panels.h"
#include "menu/base/submenus/player_movement.h"
#include "menu/base/submenus/player_appearance.h"
#include "menu/base/submenus/vehicle.h"
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
#include "menu/base/submenus/settings.h"
#include "menu/base/submenus/vehicle_acrobatics.h"
#include "menu/base/submenus/vehicle_parachute.h"
#include "menu/base/submenus/world_bullet_tracers.h"
#include "menu/base/submenus/world_trains.h"
#include "menu/base/submenus/settings_streamer.h"
#include "menu/base/submenus/vehicle_spawner.h"
#include "menu/base/submenus/handling_editor.h"
#include "game/player_valid.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "menu/base/util/control.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/stacked_display.h"
#include "menu/base/util/panels.h"
#include "menu/panels/builtin_panels.h"
#include "menu/base/util/animated_texture.h"
#include "menu/base/util/rainbow.h"
#include "menu/base/util/textures.h"
#include "util/config.h"
#include "global/ui_vars.h"
#include "platform/system_ui.h"
#include "platform/log.h"
#include "rage/invoker/natives.h"

// Crash-tracing: while the menu is open, write one line per tick phase to
// /data/Ozark/insulingtav.log. The last line on disk before a crash localises it:
//   - "overlaid-return" as the last line  -> guard fired; crash is OUTSIDE our
//     tick (the game touching our injected state) -> need the klog RIP.
//   - a phase name (input/base/render/...) as the last line -> crash is INSIDE
//     that phase of our code.
// Set to 0 for release builds.
#define INSULIN_TICK_TRACE 0

namespace menu {
    void build() {
        // Bind the string/pointer-bearing texture globals (skipped by the absent
        // .init_array), load the config file, then set up the submenu tree and
        // populate the demo. Config is loaded BEFORE the submenus so each
        // add_savable() reads the persisted value.
        global::ui::init();
        util::config::load();
        menu::textures::load();           // reads /data/Ozark/images only - no natives, safe here
        menu::submenu::handler::load();   // m_current = main_menu::get()
        main_menu::get()->load();

        // Feature submenus: load + register so their feature_update runs each frame.
        player_menu::get()->load();
        menu::submenu::handler::add_submenu(player_menu::get());
        network_menu::get()->load();
        menu::submenu::handler::add_submenu(network_menu::get());
        network_players_menu::get()->load();
        menu::submenu::handler::add_submenu(network_players_menu::get());
        network_player_menu::get()->load();
        menu::submenu::handler::add_submenu(network_player_menu::get());
        protections_menu::get()->load();
        menu::submenu::handler::add_submenu(protections_menu::get());
        teleport_menu::get()->load();
        menu::submenu::handler::add_submenu(teleport_menu::get());
        teleport_directional_menu::get()->load();
        menu::submenu::handler::add_submenu(teleport_directional_menu::get());
        teleport_ipl_menu::get()->load();
        menu::submenu::handler::add_submenu(teleport_ipl_menu::get());
        teleport_save_load_menu::get()->load();
        menu::submenu::handler::add_submenu(teleport_save_load_menu::get());
        misc_camera_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_camera_menu::get());
        misc_radio_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_radio_menu::get());
        misc_visions_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_visions_menu::get());
        misc_disables_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_disables_menu::get());
        misc_dispatch_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_dispatch_menu::get());
        weapon_explosion_gun_menu::get()->load();
        menu::submenu::handler::add_submenu(weapon_explosion_gun_menu::get());
        weapon_gravity_gun_menu::get()->load();
        menu::submenu::handler::add_submenu(weapon_gravity_gun_menu::get());
        weapon_entity_gun_menu::get()->load();
        menu::submenu::handler::add_submenu(weapon_entity_gun_menu::get());
        vehicle_colours_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_colours_menu::get());
        vehicle_neon_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_neon_menu::get());
        vehicle_plate_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_plate_menu::get());
        settings_themes_menu::get()->load();
        menu::submenu::handler::add_submenu(settings_themes_menu::get());

        // Re-apply the last saved theme now: util::config::load() has already
        // run (above) so the "LastTheme" key is in memory, and
        // settings_themes_menu::load() has just run so its name stack (parent
        // chain via set_parent<settings_menu>()) is populated - both are
        // required for get_submenu_name_stack() to resolve to the right config
        // path. Placed as early as both conditions allow, so every submenu
        // loaded afterward sees the restored colours rather than the compiled
        // defaults. Legal here: load_file only touches files and logf, no
        // natives (see theme.cpp).
        settings_themes_menu::apply_last_theme();

        helper_color_menu::get()->load();
        menu::submenu::handler::add_submenu(helper_color_menu::get());
        helper_color_presets_menu::get()->load();
        menu::submenu::handler::add_submenu(helper_color_presets_menu::get());
        helper_color_sync_menu::get()->load();
        menu::submenu::handler::add_submenu(helper_color_sync_menu::get());
        weapon_menu::get()->load();
        menu::submenu::handler::add_submenu(weapon_menu::get());
        weapon_give_menu::get()->load();
        menu::submenu::handler::add_submenu(weapon_give_menu::get());
        weapon_disables_menu::get()->load();
        menu::submenu::handler::add_submenu(weapon_disables_menu::get());
        spawner_menu::get()->load();
        menu::submenu::handler::add_submenu(spawner_menu::get());
        world_menu::get()->load();
        menu::submenu::handler::add_submenu(world_menu::get());
        world_local_entities_menu::get()->load();
        menu::submenu::handler::add_submenu(world_local_entities_menu::get());
        world_game_fx_menu::get()->load();
        menu::submenu::handler::add_submenu(world_game_fx_menu::get());
        world_weather_menu::get()->load();
        menu::submenu::handler::add_submenu(world_weather_menu::get());
        world_time_menu::get()->load();
        menu::submenu::handler::add_submenu(world_time_menu::get());
        world_clear_area_menu::get()->load();
        menu::submenu::handler::add_submenu(world_clear_area_menu::get());
        world_ocean_menu::get()->load();
        menu::submenu::handler::add_submenu(world_ocean_menu::get());
        spawner_peds_menu::get()->load();
        menu::submenu::handler::add_submenu(spawner_peds_menu::get());
        misc_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_menu::get());
        player_model_menu::get()->load();
        menu::submenu::handler::add_submenu(player_model_menu::get());
        player_wardrobe_menu::get()->load();
        menu::submenu::handler::add_submenu(player_wardrobe_menu::get());
        player_animations_menu::get()->load();
        menu::submenu::handler::add_submenu(player_animations_menu::get());
        player_scenarios_menu::get()->load();
        menu::submenu::handler::add_submenu(player_scenarios_menu::get());
        player_clipsets_menu::get()->load();
        menu::submenu::handler::add_submenu(player_clipsets_menu::get());
        player_wardrobe_saveload_menu::get()->load();
        menu::submenu::handler::add_submenu(player_wardrobe_saveload_menu::get());
        player_particles_menu::get()->load();
        menu::submenu::handler::add_submenu(player_particles_menu::get());
        player_particle_manager_menu::get()->load();
        menu::submenu::handler::add_submenu(player_particle_manager_menu::get());
        player_hand_trails_menu::get()->load();
        menu::submenu::handler::add_submenu(player_hand_trails_menu::get());
        player_animation_menu::get()->load();
        menu::submenu::handler::add_submenu(player_animation_menu::get());
        player_movement_menu::get()->load();
        menu::submenu::handler::add_submenu(player_movement_menu::get());
        player_appearance_menu::get()->load();
        menu::submenu::handler::add_submenu(player_appearance_menu::get());
        vehicle_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_menu::get());
        vehicle_customs_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_customs_menu::get());
        vehicle_health_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_health_menu::get());
        vehicle_weapons_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_weapons_menu::get());
        vehicle_particles_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_particles_menu::get());
        vehicle_movement_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_movement_menu::get());
        vehicle_boost_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_boost_menu::get());
        vehicle_collision_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_collision_menu::get());
        vehicle_gravity_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_gravity_menu::get());
        vehicle_multipliers_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_multipliers_menu::get());
        vehicle_modifiers_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_modifiers_menu::get());
        vehicle_autopilot_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_autopilot_menu::get());
        vehicle_ramps_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_ramps_menu::get());
        vehicle_randomization_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_randomization_menu::get());
        vehicle_seats_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_seats_menu::get());
        vehicle_speedometer_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_speedometer_menu::get());
        vehicle_doors_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_doors_menu::get());
        vehicle_tyre_tracks_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_tyre_tracks_menu::get());
        vehicle_spawner_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_spawner_menu::get());
        vehicle_class_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_class_menu::get());
        handling_editor_menu::get()->load();
        menu::submenu::handler::add_submenu(handling_editor_menu::get());
        vehicle_acrobatics_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_acrobatics_menu::get());
        vehicle_parachute_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_parachute_menu::get());
        world_bullet_tracers_menu::get()->load();
        menu::submenu::handler::add_submenu(world_bullet_tracers_menu::get());
        world_trains_menu::get()->load();
        menu::submenu::handler::add_submenu(world_trains_menu::get());
        settings_streamer_menu::get()->load();
        menu::submenu::handler::add_submenu(settings_streamer_menu::get());
        settings_menu::get()->load();
        menu::submenu::handler::add_submenu(settings_menu::get());
        language_menu::get()->load();
        menu::submenu::handler::add_submenu(language_menu::get());

        menu::panels::register_builtin_panels();

        // Panels registered above so this load() has children to read from.
        misc_panels_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_panels_menu::get());

        // Kept: one line, once, and it is the proof that the whole tree got
        // built without taking the game down. Paired with the BUILD= line it
        // says which .prx ran and how far it got.
        platform::klogf("boot: build done, %d submenus",
                        (int)menu::submenu::handler::get_submenus().size());
    }

    static int g_hash_tries = 0;   // attempts at recovering the native table

    void tick() {
        bool open = menu::base::is_open();
        bool ov = platform::system_ui::overlaid();

#if INSULIN_TICK_TRACE
    // Dual sink so we capture the crash window no matter what: klog shows up live
    // in `nc <ip> 3232` (no FTP), and /data/Ozark/insulingtav.log is the guaranteed
    // backup if plugin klog output doesn't reach the broadcast. Only while the
    // menu is open (the sole crash condition), so the volume stays bounded.
    #define TICK_TRACE(p) do { if (open) { \
            platform::klogf("tick:%s ov=%d", p, (int)ov); \
            platform::logf("tick", "%s ov=%d", p, (int)ov); \
        } } while (0)
#else
    #define TICK_TRACE(p) do { } while (0)
#endif

        TICK_TRACE("enter");

        // PS-button guard: while the ShellUI overlay (XMB) is up the game is
        // constrained and our per-frame native work crashes it. Skip the whole
        // tick - not just rendering - so no native is touched until the game
        // has the screen again. Features pause for the overlay's duration,
        // which is invisible: the game is paused under the overlay anyway.
        if (ov) { TICK_TRACE("overlaid-return"); return; }

        // g_delta drives the scroller lerp; refresh it from the frame time.
        global::ui::g_delta = native::get_frame_time();

        // Step every loaded animation before anything draws, so update and render
        // stay separate and the renderer keeps no side effects.
        menu::animation::update(global::ui::g_delta);

        // Input first (open bind L1+O + navigation), then base (control-disable +
        // render + handler update), then drain any deferred menu-input actions.
        TICK_TRACE("input");
        menu::input::update();
        TICK_TRACE("base");
        menu::base::update();
        TICK_TRACE("mi_update");
        menu::input::mi_update();

        // Per-frame feature loop (godmode etc. re-applied every frame, whether or
        // not the owning submenu is open), then the control manager's request
        // queues (model/asset streaming for spawns).
        // Features touch the game through natives, so they only run once the local
        // player actually exists. Ozark gates the same way (it waits for
        // GameStatePlaying before init); without a gate a feature firing during the
        // loading screen dereferences a player that is not there yet.
        // The game's native table is empty when the plugin loads - GoldHEN gets
        // us in before the script system registers anything - so it is recovered
        // here instead, once the game is actually up. Retried a few times because
        // "the player exists" and "every native is registered" are not the same
        // moment; each attempt is a cheap walk of 256 buckets.
        if (game::player_valid() && !rage::hash_natives::usable() && g_hash_tries < 10) {
            if ((native::get_frame_count() % 120) == 0) {
                g_hash_tries++;
                rage::hash_natives::build();
            }
        }

        TICK_TRACE("feature_update");
        if (game::player_valid())
            menu::submenu::handler::feature_update();
        TICK_TRACE("control");
        menu::control::update();

        // No player_valid() gate: this reads and writes plain memory and calls
        // no natives. It cannot run during build() either, because tick is only
        // wired as the frame callback after build() returns.
        menu::get_rainbow()->run();

        // Notifications + stacked display render every frame.
        TICK_TRACE("notify");
        menu::notify::update();
        TICK_TRACE("display");
        menu::display::render();

        TICK_TRACE("panels");
        // Gated like feature_update: panel callbacks call natives, and before the
        // local player exists those dereference a player that is not there. The
        // gate lives here rather than in each callback so it also covers every
        // panel written from now on.
        if (game::player_valid())
            menu::panels::update();
        TICK_TRACE("done");

#undef TICK_TRACE
    }
}
