#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Game FX
class world_game_fx_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_game_fx_menu* get();
};
