#pragma once
#include "menu/base/submenu.h"

// Visibility and the cosmetic state of the ped - blood, dirt, wetness, helmet.
class player_appearance_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_appearance_menu* get();
};
