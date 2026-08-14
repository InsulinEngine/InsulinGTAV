#pragma once
#include "menu/base/submenu.h"

// Ozark: Weapon > Disables
class weapon_disables_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static weapon_disables_menu* get();
};
