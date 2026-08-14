#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Train
class world_trains_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_trains_menu* get();
};
