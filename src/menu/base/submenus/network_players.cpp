#include "menu/base/submenus/network_players.h"
#include "menu/base/submenus/network.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/esp.h"
#include "menu/base/submenus/helper_esp.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"
#include "game/player_list.h"

// Ozark: Network > Players. The list is rebuilt on entry rather than at load,
// because who is in the session changes constantly - a list built once would show
// whoever happened to be there when the menu was created.

namespace {
    int g_selected = -1;   // player the child menu acts on

    // One blip per player slot, so turning blips off can remove exactly the ones
    // this menu created rather than clearing the map.
    Blip g_blips[game::players::MAX_PLAYERS] = {};
    bool g_show_blips = false;
    bool g_spectating = false;

    // m_ped = true: every entity this context draws is a player ped, so the
    // skeleton and weapon elements are available from the start.
    menu::esp::esp_context g_session_esp = { true };

    // One context per slot rather than one shared: the point of the per-player
    // menu is that two players can be marked differently. esp_context is an
    // aggregate of constant-initialisable members (color_rgba's constructors
    // are constexpr - see ui_vars.h), so this array is constant-initialised
    // and needs no .init_array entry to come up correctly.
    menu::esp::esp_context g_player_esp[game::players::MAX_PLAYERS] = {};
}

// The selected player is shared with the per-player menu below.
int network_players_selected() { return g_selected; }

void network_players_menu::load() {
    set_name("Players");
    set_parent<network_menu>();
    update_once();
}

void network_players_menu::update_once() {
    clear_options(0);

    if (!game::players::in_session()) {
        add_option(button_option("~m~No active session").ref());
        return;
    }

    add_option(toggle_option("Show Blips")
        .add_toggle(g_show_blips)
        .add_tooltip("Marks every other player on the map"));

    add_option(submenu_option("ESP")
        .add_submenu<helper_esp_menu>()
        .add_click([] {
            helper_esp_menu::open_for<network_players_menu>(&g_session_esp, "Session ESP");
        })
        .add_tooltip("Draws every other player in the session"));

    add_option(break_option("Players").ref());

    int shown = 0;
    for (int i = 0; i < game::players::MAX_PLAYERS; i++) {
        if (!game::players::valid(i))
            continue;

        game::players::entry e = game::players::get(i);
        int id = i;   // tiny capture: stl::function caps captures at 64 bytes

        add_option(submenu_option(e.name && e.name[0] ? e.name : "(unnamed)")
            .add_submenu<network_player_menu>()
            .add_click([id] { g_selected = id; }));
        shown++;
    }

    if (!shown)
        add_option(button_option("~m~No players").ref());
}

void network_players_menu::feature_update() {
    // Drawn from feature_update rather than update, so the ESP stays up with
    // the menu closed - which is the only time it is useful.
    if (g_session_esp.any() && game::players::in_session()) {
        const int me = game::players::local_id();
        for (int i = 0; i < game::players::MAX_PLAYERS; i++) {
            if (i == me || !game::players::valid(i)) continue;
            game::players::entry e = game::players::get(i);
            if (!e.ped) continue;
            menu::esp::draw_entity(g_session_esp, e.ped, e.name);
        }
    }

    // Blips are created once per player and removed when the toggle goes off or
    // the player leaves - recreating them every frame would stack thousands.
    for (int i = 0; i < game::players::MAX_PLAYERS; i++) {
        bool want = g_show_blips && game::players::valid(i) && i != game::players::local_id();

        if (want && !g_blips[i]) {
            Ped ped = native::get_player_ped(i);
            if (ped) {
                g_blips[i] = native::add_blip_for_entity(ped);
                native::set_blip_sprite(g_blips[i], 1);
                native::set_blip_colour(g_blips[i], 2);
                native::set_blip_scale(g_blips[i], 0.9f);
            }
        } else if (!want && g_blips[i]) {
            if (native::does_blip_exist(g_blips[i]))
                native::remove_blip(&g_blips[i]);
            g_blips[i] = 0;
        }
    }
}

void network_players_menu::update() {
    // Rebuilt every time the count changes, so someone joining or leaving is
    // reflected without leaving a dead entry that acts on a freed player.
    static int last = -1;
    int now = game::players::count();
    if (now != last) {
        last = now;
        update_once();
    }
}

network_players_menu* network_players_menu::get() {
    static network_players_menu instance;
    return &instance;
}

// ---- one player ---------------------------------------------------------------
void network_player_menu::load() {
    set_name("Player");
    set_parent<network_players_menu>();

    add_option(button_option("Teleport To")
        .add_click([] {
            game::players::entry e = game::players::get(g_selected);
            if (!e.ped) return;
            math::vector3<float> c = native::get_entity_coords(e.ped, true);
            Ped me = native::get_player_ped(-1);
            native::set_entity_coords_no_offset(me, c.x, c.y, c.z + 1.f, false, false, false);
        }));

    add_option(button_option("Bring To Me")
        .add_click([] {
            game::players::entry e = game::players::get(g_selected);
            if (!e.ped) return;
            math::vector3<float> c = native::get_entity_coords(native::get_player_ped(-1), true);
            // Only works on a player the local machine owns; on a remote one the
            // move is rejected, which is why nothing is reported as success here.
            native::set_entity_coords_no_offset(e.ped, c.x, c.y + 2.f, c.z, false, false, false);
        }));

    add_option(button_option("Spectate")
        .add_tooltip("Watch this player. Press again to stop.")
        .add_click([] {
            game::players::entry e = game::players::get(g_selected);
            if (!e.ped) return;
            g_spectating = !g_spectating;
            native::network_set_in_spectator_mode(g_spectating, e.ped);
            menu::notify::stacked("Player", g_spectating ? "Spectating" : "Stopped");
        }));

    add_option(submenu_option("ESP")
        .add_submenu<helper_esp_menu>()
        .add_click([] {
            int id = network_players_selected();
            if (id < 0 || id >= game::players::MAX_PLAYERS) return;
            g_player_esp[id].m_ped = true;
            helper_esp_menu::open_for<network_player_menu>(&g_player_esp[id], "Player ESP");
        }));

    add_option(break_option("Info").ref());

    add_option(button_option("Show Details")
        .add_click([] {
            game::players::entry e = game::players::get(g_selected);
            if (!e.ped) {
                menu::notify::stacked("Player", "Gone");
                return;
            }
            menu::notify::stacked(e.name, e.alive ? "Alive" : "Dead");
        }));
}

void network_player_menu::update_once() {
    game::players::entry e = game::players::get(g_selected);
    set_name(e.name && e.name[0] ? e.name : "Player");
}

void network_player_menu::feature_update() {
    // Same gate as the parent menu's sweep: outside a session the per-slot
    // contexts describe players who are not there, and every valid() call below
    // is a native asked about a slot that cannot be occupied.
    if (!game::players::in_session()) return;

    for (int i = 0; i < game::players::MAX_PLAYERS; i++) {
        if (!g_player_esp[i].any() || !game::players::valid(i)) continue;
        if (i == game::players::local_id()) continue;
        game::players::entry e = game::players::get(i);
        if (e.ped) menu::esp::draw_entity(g_player_esp[i], e.ped, e.name);
    }
}

network_player_menu* network_player_menu::get() {
    static network_player_menu instance;
    return &instance;
}
