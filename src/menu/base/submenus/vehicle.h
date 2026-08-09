#pragma once
#include "menu/base/submenu.h"

// Vehicle submenu: validates the control baustein — spawns request their model
// through control::request_model and create the vehicle in the load callback.
class vehicle_menu : public menu::submenu::submenu {
public:
    static vehicle_menu* get();
    void load() override;
    vehicle_menu() : menu::submenu::submenu() {}
};
