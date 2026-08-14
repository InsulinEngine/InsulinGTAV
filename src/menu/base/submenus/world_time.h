#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Time
class world_time_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_time_menu* get();
};
