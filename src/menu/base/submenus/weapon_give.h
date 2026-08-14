#pragma once
#include "menu/base/submenu.h"

// Ozark: Weapon > Give Weapons and Ammo
class weapon_give_menu : public menu::submenu::submenu {
public:
    void load() override;
    static weapon_give_menu* get();
};
