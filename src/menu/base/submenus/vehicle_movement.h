#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Movement
class vehicle_movement_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_movement_menu* get();
};
