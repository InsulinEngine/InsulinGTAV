#include "menu/base/submenus/vehicle_spawner.h"
#include "menu/base/submenus/spawner.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/control.h"
#include "menu/base/util/notify.h"
#include "menu/base/submenu_handler.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "game/vehicle_list.h"
#include "platform/log.h"

// The generated list is a flat array sorted by class, so a class submenu is just the
// contiguous run of entries whose cls matches. Nothing is copied or allocated.

namespace {
    int  g_class = 0;       // class chosen in the spawner menu
    int  g_built = -1;      // class the list submenu currently shows
    bool g_warp = true;     // put the player in the vehicle after spawning
    bool g_engine = true;   // start the engine
    bool g_upgrade = false; // fit maximum performance mods

    // stl has no to_string; this is only ever used for small counts.
    stl::string itos(int v) {
        char digits[12];
        int n = 0;
        if (v <= 0) {
            digits[n++] = '0';
        } else {
            while (v > 0 && n < 11) { digits[n++] = (char)('0' + (v % 10)); v /= 10; }
        }
        char out[12];
        for (int i = 0; i < n; i++) out[i] = digits[n - 1 - i];
        out[n] = '\0';
        return stl::string(out);
    }

    // Spawn in front of the player. The model has to be streamed in first; the
    // control manager hands us the loaded model in its callback, which is the same
    // workflow the game's own spawn paths use.
    void spawn_model(const char* model_name) {
        uint32_t model = native::get_hash_key(model_name);

        if (!native::is_model_in_cdimage(model) || !native::is_model_a_vehicle(model)) {
            menu::notify::stacked("Spawner", "Model not available on this build");
            return;
        }

        menu::control::request_model(model, [model](uint32_t loaded) {
            Ped ped = native::get_player_ped(-1);
            math::vector3<float> pos = native::get_offset_from_entity_in_world_coords(ped, 0.f, 5.f, 0.f);
            float heading = native::get_entity_heading(ped);

            Vehicle veh = native::create_vehicle(loaded, pos.x, pos.y, pos.z, heading, false, false, 0);
            native::set_vehicle_on_ground_properly(veh, 0);

            // Without this the population manager is free to clean the vehicle up
            // again, which yanks the player out with it.
            native::set_entity_as_mission_entity(veh, true, true);

            if (g_engine)
                native::set_vehicle_engine_on(veh, true, true, false);

            // Needed before any SET_VEHICLE_MOD call works. It does NOT stop the
            // game from putting you back out of DLC vehicles in Story Mode - that
            // one is shop_controller clearing the ped's tasks, unrelated to mods.
            native::set_vehicle_mod_kit(veh, 0);

            if (g_upgrade) {
                for (int slot = 11; slot <= 16; slot++) {   // engine..turbo range
                    int count = native::get_num_vehicle_mods(veh, slot);
                    if (count > 0)
                        native::set_vehicle_mod(veh, slot, count - 1, false);
                }
                native::toggle_vehicle_mod(veh, 18, true);  // turbo
            }

            if (g_warp) {
                Ped me = native::get_player_ped(-1);
                // Warping does not touch the ped's task stack. Whatever it was doing
                // when you opened the menu (walking, running) resumes a moment later
                // and walks it straight back out of the seat, so clear it first.
                native::clear_ped_tasks_immediately(me);
                native::set_ped_into_vehicle(me, veh, -1);
            }

            native::set_model_as_no_longer_needed(model);
            menu::notify::stacked("Spawner", "Spawned");
        });
    }
}

// ---- class list -------------------------------------------------------------
void vehicle_spawner_menu::load() {
    set_name("Vehicles");
    set_parent<spawner_menu>();

    add_option(toggle_option("Warp Into Vehicle")
        .add_toggle(g_warp)
        .add_tooltip("Sit in the vehicle right after it spawns")
        .add_savable(get_submenu_name_stack()));
    add_option(toggle_option("Engine Running")
        .add_toggle(g_engine)
        .add_tooltip("Start the engine on spawn")
        .add_savable(get_submenu_name_stack()));
    add_option(toggle_option("Max Performance Mods")
        .add_toggle(g_upgrade)
        .add_tooltip("Fit the best engine/brakes/transmission/suspension and a turbo")
        .add_savable(get_submenu_name_stack()));


    add_option(break_option("Classes").ref());

    for (int i = 0; i < vehicle_class_count; i++) {
        int count = 0;
        for (int v = 0; v < vehicle_list_count; v++)
            if (vehicle_list[v].cls == i) count++;
        if (count == 0)
            continue;

        int cls = i;    // tiny capture: stl::function caps captures at 64 bytes
        add_option(submenu_option(vehicle_class_names[i])
            .add_submenu<vehicle_class_menu>()
            .add_side_text(itos(count))
            .add_click([cls] { g_class = cls; }));
    }
}

vehicle_spawner_menu* vehicle_spawner_menu::get() {
    static vehicle_spawner_menu instance;
    return &instance;
}

// ---- vehicles of the selected class -----------------------------------------
void vehicle_class_menu::load() {
    set_name("Vehicles");
    set_parent<vehicle_spawner_menu>();
    update_once();
}

void vehicle_class_menu::update() {
    // The click that picks a class and the navigation into this submenu are two
    // separate steps, so rebuild here rather than relying on their ordering.
    if (g_built != g_class)
        update_once();
}

void vehicle_class_menu::update_once() {
    g_built = g_class;
    clear_options(0);

    if (g_class < 0 || g_class >= vehicle_class_count) {
        add_option(button_option("~m~(no class selected)").ref());
        return;
    }

    set_name(vehicle_class_names[g_class]);

    for (int i = 0; i < vehicle_list_count; i++) {
        if (vehicle_list[i].cls != g_class)
            continue;

        int idx = i;    // tiny capture only
        button_option opt(vehicle_list[i].name);
        if (vehicle_list[i].make[0])
            opt.add_tooltip(stl::string(vehicle_list[i].make) + " - " + vehicle_list[i].model);
        else
            opt.add_tooltip(vehicle_list[i].model);

        add_option(opt.add_click([idx] { spawn_model(vehicle_list[idx].model); }));
    }
}

vehicle_class_menu* vehicle_class_menu::get() {
    static vehicle_class_menu instance;
    return &instance;
}
