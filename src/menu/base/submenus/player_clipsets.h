#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Animation > Clipset. Movement and weapon clipsets.
class player_clipsets_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static player_clipsets_menu* get();
};
