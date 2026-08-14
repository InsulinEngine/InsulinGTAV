#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Bullet Tracers
class world_bullet_tracers_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_bullet_tracers_menu* get();
};
