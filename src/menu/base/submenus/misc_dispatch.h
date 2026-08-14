#pragma once
#include "menu/base/submenu.h"

// Ozark: Miscellaneous > Dispatch
class misc_dispatch_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static misc_dispatch_menu* get();
};
