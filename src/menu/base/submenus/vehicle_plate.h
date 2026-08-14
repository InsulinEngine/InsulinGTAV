#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Number Plate
class vehicle_plate_menu : public menu::submenu::submenu {
public:
    void load() override;
    static vehicle_plate_menu* get();
};
