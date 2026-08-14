#pragma once
#include "menu/base/submenu.h"

// Ozark: Weapon > Aim Assist
class weapon_aimbot_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static weapon_aimbot_menu* get();
};
