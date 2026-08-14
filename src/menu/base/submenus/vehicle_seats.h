#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Seats
class vehicle_seats_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_seats_menu* get();
};
