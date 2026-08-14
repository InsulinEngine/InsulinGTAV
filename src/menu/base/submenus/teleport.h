#pragma once
#include "menu/base/submenu.h"

// Getting somewhere. Waypoint and blip teleports read the map, the rest are
// fixed landmarks.
class teleport_menu : public menu::submenu::submenu {
public:
    void load() override;
    static teleport_menu* get();
};
