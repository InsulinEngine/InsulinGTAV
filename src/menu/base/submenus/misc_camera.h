#pragma once
#include "menu/base/submenu.h"

// Ozark: Miscellaneous > Camera
class misc_camera_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static misc_camera_menu* get();
};
