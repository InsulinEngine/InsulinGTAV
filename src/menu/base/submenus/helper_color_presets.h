#pragma once
#include "menu/base/submenu.h"

class helper_color_presets_menu : public menu::submenu::submenu {
public:
    void load() override;
    static helper_color_presets_menu* get();
};
