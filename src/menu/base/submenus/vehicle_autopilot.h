#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Autopilot
class vehicle_autopilot_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_autopilot_menu* get();
};
