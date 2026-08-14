#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Tire Tracks
class vehicle_tyre_tracks_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_tyre_tracks_menu* get();
};
