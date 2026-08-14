#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Weapons
class vehicle_weapons_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_weapons_menu* get();
};
