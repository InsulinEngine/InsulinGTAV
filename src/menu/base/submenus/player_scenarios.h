#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Animation > Scenarios. Scenarios need no streaming - the game
// already owns the assets.
class player_scenarios_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_scenarios_menu* get();
};
