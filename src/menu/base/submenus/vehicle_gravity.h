#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Gravity
class vehicle_gravity_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_gravity_menu* get();
};
