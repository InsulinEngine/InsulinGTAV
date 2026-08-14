#pragma once
#include "menu/base/submenu.h"

// Ozark: Weapon > Entity Gun
class weapon_entity_gun_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static weapon_entity_gun_menu* get();
};
