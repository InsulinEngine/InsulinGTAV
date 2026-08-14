#include "menu/base/submenus/network.h"
#include "menu/base/submenus/main.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/submenus/network_players.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"
#include "platform/log.h"

// Everything here only means something once a session is up. In Story Mode the
// readouts report "no session" and the toggles do nothing, which is the honest
// behaviour - the alternative would be hiding the menu and leaving you guessing
// whether the plugin sees the session at all.

namespace {
    bool g_stay_off_radar = false;

    // stl has no to_string.
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
}

void network_menu::load() {
    set_name("Network");
    set_parent<main_menu>();

    add_option(submenu_option("Players").add_submenu<network_players_menu>()
        .add_tooltip("Everyone in the current session"));

    add_option(break_option("Session").ref());

    add_option(button_option("Session Info")
        .add_tooltip("Reports whether the game considers itself in a session")
        .add_click([] {
            if (!native::network_is_session_active()) {
                menu::notify::stacked("Network", "No active session");
                return;
            }
            stl::string msg = itos(native::network_get_num_connected_players()) + " players";
            menu::notify::stacked("Network", msg.c_str());
        }));

    add_option(button_option("Am I Host")
        .add_click([] {
            menu::notify::stacked("Network",
                native::network_is_host() ? "You are the host" : "Not the host");
        }));

    add_option(break_option("Presence").ref());

    add_option(toggle_option("Off The Radar")
        .add_toggle(g_stay_off_radar)
        .add_tooltip("Hides your blip from other players. Session only.")
        .add_savable(get_submenu_name_stack()));
}

void network_menu::feature_update() {
    // Guard on the session: pushing presence flags outside one is pointless work
    // every frame, and on some builds it is how you get a stuck flag afterwards.
    if (!native::network_is_session_active())
        return;

    if (g_stay_off_radar)
        native::network_set_in_free_cam_mode(true);
}

network_menu* network_menu::get() {
    static network_menu instance;
    return &instance;
}
