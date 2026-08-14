#pragma once
#include "menu/base/submenu.h"

// Movement and stamina. The multipliers are the game's own player modifiers, which
// it resets on its own schedule, so they are pushed every frame rather than once.
class player_movement_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static player_movement_menu* get();
};
