#pragma once
#include "menu/base/submenu.h"

// Ozark: Weapon > Gravity Gun
class weapon_gravity_gun_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static weapon_gravity_gun_menu* get();
};
