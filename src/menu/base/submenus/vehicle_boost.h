#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Boost
class vehicle_boost_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_boost_menu* get();
};
