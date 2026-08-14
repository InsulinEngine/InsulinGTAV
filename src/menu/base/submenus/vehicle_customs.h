#pragma once
#include "menu/base/submenu.h"

// Ozark: Vehicle > Customs
class vehicle_customs_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static vehicle_customs_menu* get();
};
