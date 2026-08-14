#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Weather
class world_weather_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_weather_menu* get();
};
