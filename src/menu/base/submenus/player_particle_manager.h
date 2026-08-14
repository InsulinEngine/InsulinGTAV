#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Particle FX > Particle Manager.
class player_particle_manager_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_particle_manager_menu* get();
};
