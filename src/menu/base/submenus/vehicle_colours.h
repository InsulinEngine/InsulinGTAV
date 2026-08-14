#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Colours
class vehicle_colours_menu : public menu::submenu::submenu {
public:
    void load() override;
    static vehicle_colours_menu* get();
};
