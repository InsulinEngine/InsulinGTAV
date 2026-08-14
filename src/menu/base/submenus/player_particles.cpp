#include "menu/base/submenus/player_particles.h"
#include "menu/base/submenus/player.h"
#include "menu/base/submenus/player_particle_manager.h"
#include "menu/base/submenus/player_hand_trails.h"
#include "menu/base/options/submenu_option.h"

void player_particles_menu::load() {
    set_name("Particle FX");
    set_parent<player_menu>();

    add_option(submenu_option("Particle Manager").add_submenu<player_particle_manager_menu>());
    add_option(submenu_option("Hand Trails").add_submenu<player_hand_trails_menu>());
}

player_particles_menu* player_particles_menu::get() {
    static player_particles_menu instance;
    return &instance;
}
