#include "menu/base/submenus/player_hand_trails.h"
#include "menu/base/submenus/player_particles.h"
#include "menu/base/options/toggle.h"
#include "rage/invoker/natives.h"

namespace {
    bool g_trails = false;
}

void player_hand_trails_menu::load() {
    set_name("Hand Trails");
    set_parent<player_particles_menu>();

    add_option(toggle_option("Toggle Hand Trails")
        .add_toggle(g_trails)
        .add_tooltip("Needs the PTFX asset list and a colour picker, neither ported yet")
        .add_savable(get_submenu_name_stack()));
}

player_hand_trails_menu* player_hand_trails_menu::get() {
    static player_hand_trails_menu instance;
    return &instance;
}
