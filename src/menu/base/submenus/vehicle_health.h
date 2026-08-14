#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Health
class vehicle_health_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_health_menu* get();
};
