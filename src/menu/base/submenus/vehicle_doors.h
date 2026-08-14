#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Doors
class vehicle_doors_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_doors_menu* get();
};
