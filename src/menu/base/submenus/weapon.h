#pragma once
#include "menu/base/submenu.h"

// Ammo, damage behaviour and the weapon-modifier natives that have to be pushed
// every frame.
class weapon_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static weapon_menu* get();
};
