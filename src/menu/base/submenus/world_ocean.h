#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Ocean
class world_ocean_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_ocean_menu* get();
};
