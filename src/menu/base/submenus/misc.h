#pragma once
#include "menu/base/submenu.h"

// HUD and things that fit nowhere else.
class misc_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static misc_menu* get();
};
