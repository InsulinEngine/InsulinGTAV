#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Acrobatics
class vehicle_acrobatics_menu : public menu::submenu::submenu {
public:
    void load() override;
    static vehicle_acrobatics_menu* get();
};
