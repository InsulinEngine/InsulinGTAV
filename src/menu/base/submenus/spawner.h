#pragma once
#include "menu/base/submenu.h"

// Top-level spawner, mirroring Ozark. Vehicles live in their own submenu.
class spawner_menu : public menu::submenu::submenu {
public:
    void load() override;
    static spawner_menu* get();
};
