#include "menu/base/submenus/player_particle_manager.h"
#include "menu/base/submenus/player_particles.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"

namespace {
    bool g_looped = false;
}

void player_particle_manager_menu::load() {
    set_name("Particle Manager");
    set_parent<player_particles_menu>();

    add_option(button_option("Stop Particles")
        .add_click([] {
            native::remove_particle_fx_in_range(0.f, 0.f, 0.f, 100000.f);
            menu::notify::stacked("Particles", "Stopped");
        }));

    add_option(toggle_option("Looped")
        .add_toggle(g_looped)
        .add_tooltip("Ozark plays the selected effect on a loop. The effect picker "
                     "needs the PTFX asset lists, which are not ported yet.")
        .add_savable(get_submenu_name_stack()));
}

player_particle_manager_menu* player_particle_manager_menu::get() {
    static player_particle_manager_menu instance;
    return &instance;
}
