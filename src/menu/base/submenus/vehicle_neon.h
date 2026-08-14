#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Neon
class vehicle_neon_menu : public menu::submenu::submenu {
public:
    void load() override;
    static vehicle_neon_menu* get();
};
