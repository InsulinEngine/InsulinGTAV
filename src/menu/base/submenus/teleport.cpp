#include "menu/base/submenus/teleport.h"
#include "menu/base/submenus/main.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/submenus/teleport_directional.h"
#include "menu/base/submenus/teleport_ipl.h"
#include "menu/base/submenus/teleport_save_load.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"
#include "game/teleport_actions.h"

namespace {
    void warp(float x, float y, float z) {
        Entity e = game::teleport_subject();
        if (!e)
            return;
        native::set_entity_coords_no_offset(e, x, y, z, false, false, false);
        menu::notify::stacked("Teleport", "Moved");
    }

    struct place { const char* name; float x, y, z; };
    const place g_places[] = {
        { "Michaels House",     -852.4f,   160.0f,  65.6f },
        { "Franklins House",     -14.1f, -1440.0f,  31.1f },
        { "Trevors Trailer",    1985.7f,  3812.2f,  32.2f },
        { "Los Santos Customs", -337.2f,  -136.6f,  39.0f },
        { "LS Airport",        -1037.7f, -2737.8f,  20.2f },
        { "Mount Chiliad",       501.5f,  5604.4f, 797.9f },
        { "Maze Bank Roof",      -75.0f,  -818.6f, 326.2f },
        { "Military Base",     -2047.4f,  3132.1f,  32.8f },
    };
    constexpr int PLACE_COUNT = (int)(sizeof(g_places) / sizeof(g_places[0]));
}

void teleport_menu::load() {
    set_name("Teleport");
    set_parent<main_menu>();

    add_option(submenu_option("Save and Load").add_submenu<teleport_save_load_menu>());
    add_option(submenu_option("Directional").add_submenu<teleport_directional_menu>());
    add_option(submenu_option("IPL").add_submenu<teleport_ipl_menu>());

    add_option(break_option("Markers").ref());

    add_option(button_option("Waypoint")
        .add_tooltip("Set a marker on the map first")
        .add_click([] {
            if (!native::is_waypoint_active()) {
                menu::notify::stacked("Teleport", "No waypoint set");
                return;
            }
            // Blip 8 is the waypoint. Its coordinates carry no ground height, so
            // arrive from above and let the game drop you in.
            Blip blip = native::get_first_blip_info_id(8);
            math::vector3<float> c = native::get_blip_info_id_coord(blip);
            warp(c.x, c.y, c.z + 100.f);
        }));

    add_option(button_option("Objective Marker")
        .add_tooltip("The current mission or destination blip")
        .add_click([] {
            Blip blip = native::get_first_blip_info_id(1);
            if (!blip) {
                menu::notify::stacked("Teleport", "No objective blip");
                return;
            }
            math::vector3<float> c = native::get_blip_info_id_coord(blip);
            warp(c.x, c.y, c.z + 100.f);
        }));

    add_option(break_option("Nudge").ref());

    add_option(button_option("Up 10m")
        .add_click([] {
            Entity e = game::teleport_subject();
            if (!e) return;
            math::vector3<float> c = native::get_entity_coords(e, true);
            warp(c.x, c.y, c.z + 10.f);
        }));

    add_option(button_option("Forward 20m")
        .add_click([] {
            Entity e = game::teleport_subject();
            if (!e) return;
            math::vector3<float> c = native::get_entity_coords(e, true);
            math::vector3<float> f = native::get_entity_forward_vector(e);
            warp(c.x + f.x * 20.f, c.y + f.y * 20.f, c.z);
        }));

    add_option(break_option("Places").ref());

    for (int i = 0; i < PLACE_COUNT; i++) {
        int idx = i;    // tiny capture: stl::function caps captures at 64 bytes
        add_option(button_option(g_places[i].name)
            .add_click([idx] { warp(g_places[idx].x, g_places[idx].y, g_places[idx].z); }));
    }
}

teleport_menu* teleport_menu::get() {
    static teleport_menu instance;
    return &instance;
}
