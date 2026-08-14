#pragma once
#include "menu/base/submenu.h"

// Ozark: Teleport > IPL
class teleport_ipl_menu : public menu::submenu::submenu {
public:
    void load() override;
    static teleport_ipl_menu* get();
};
