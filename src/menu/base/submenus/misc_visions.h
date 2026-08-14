#pragma once
#include "menu/base/submenu.h"

// Ozark: Miscellaneous > Visions
class misc_visions_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static misc_visions_menu* get();
};
