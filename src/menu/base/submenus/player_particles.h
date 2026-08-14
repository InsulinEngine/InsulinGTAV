#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Particle FX.
class player_particles_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_particles_menu* get();
};
