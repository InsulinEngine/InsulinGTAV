#pragma once
#include "menu/base/submenu.h"

// Ozark: Miscellaneous > Radio
class misc_radio_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static misc_radio_menu* get();
};
