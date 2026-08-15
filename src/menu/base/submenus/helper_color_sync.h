#pragma once
#include "menu/base/submenu.h"

class helper_color_sync_menu : public menu::submenu::submenu {
public:
    void load() override;
    static helper_color_sync_menu* get();
};
