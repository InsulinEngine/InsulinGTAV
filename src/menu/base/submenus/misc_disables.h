#pragma once
#include "menu/base/submenu.h"

// Ozark: Miscellaneous > Disables
class misc_disables_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static misc_disables_menu* get();
};
