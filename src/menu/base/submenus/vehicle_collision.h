#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Collision
class vehicle_collision_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_collision_menu* get();
};
