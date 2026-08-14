#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Parachute
class vehicle_parachute_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_parachute_menu* get();
};
