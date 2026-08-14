#pragma once
#include "menu/base/submenu.h"

// Ozark: Teleport > Save and Load
class teleport_save_load_menu : public menu::submenu::submenu {
public:
    void load() override;
    static teleport_save_load_menu* get();
};
