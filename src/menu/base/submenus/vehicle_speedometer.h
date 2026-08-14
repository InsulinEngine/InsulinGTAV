#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Speedometer
class vehicle_speedometer_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_speedometer_menu* get();
};
