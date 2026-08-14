#pragma once
#include "menu/base/submenu.h"

// Ozark: Weapon > Explosion Gun
class weapon_explosion_gun_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static weapon_explosion_gun_menu* get();
};
