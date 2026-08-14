#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Ramps
class vehicle_ramps_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_ramps_menu* get();
};
