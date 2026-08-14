#pragma once
#include "menu/base/submenu.h"

// Ozark: World > Local Entities
class world_local_entities_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static world_local_entities_menu* get();
};
