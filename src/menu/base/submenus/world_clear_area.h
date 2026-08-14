#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Clear Area
class world_clear_area_menu : public menu::submenu::submenu {
public:
    void load() override;
    static world_clear_area_menu* get();
};
