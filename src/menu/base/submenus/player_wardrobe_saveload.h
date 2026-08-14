#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Appearance > Wardrobe > Save and Load.
class player_wardrobe_saveload_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_wardrobe_saveload_menu* get();
};
