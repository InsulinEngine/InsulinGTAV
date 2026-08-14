#pragma once
#include "menu/base/submenu.h"

// Ozark: Settings > Themes
class settings_themes_menu : public menu::submenu::submenu {
public:
    void load() override;
    static settings_themes_menu* get();
};
