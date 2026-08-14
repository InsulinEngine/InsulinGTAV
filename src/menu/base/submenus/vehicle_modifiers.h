#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Modifiers
class vehicle_modifiers_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_modifiers_menu* get();
};
