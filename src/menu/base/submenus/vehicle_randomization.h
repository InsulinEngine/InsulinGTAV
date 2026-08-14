#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Randomization
class vehicle_randomization_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_randomization_menu* get();
};
