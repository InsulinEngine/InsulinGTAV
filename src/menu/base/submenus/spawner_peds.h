#pragma once
#include "menu/base/submenu.h"

// Ozark: Spawner > Peds
class spawner_peds_menu : public menu::submenu::submenu {
public:
    void load() override;
    static spawner_peds_menu* get();
};
