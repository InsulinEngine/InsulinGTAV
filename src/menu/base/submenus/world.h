#pragma once
#include "menu/base/submenu.h"

// Weather, time and how busy the world is.
class world_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_menu* get();
};
