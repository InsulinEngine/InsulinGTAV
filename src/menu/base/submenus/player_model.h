#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Appearance > Model. Seven scroll lists rather than one submenu
// per category, which is how Ozark presents it.
class player_model_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static player_model_menu* get();
};
