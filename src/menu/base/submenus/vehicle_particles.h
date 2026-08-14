#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Particle FX
class vehicle_particles_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_particles_menu* get();
};
