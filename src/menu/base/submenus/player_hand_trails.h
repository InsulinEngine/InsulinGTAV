#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Particle FX > Hand Trails.
class player_hand_trails_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_hand_trails_menu* get();
};
