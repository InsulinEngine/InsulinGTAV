#pragma once
#include "menu/base/submenu.h"

// Ozark: Teleport > Directional
class teleport_directional_menu : public menu::submenu::submenu {
public:
    void load() override;
    static teleport_directional_menu* get();
};
