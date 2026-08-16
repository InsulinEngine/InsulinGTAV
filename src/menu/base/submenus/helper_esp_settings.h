#pragma once
#include "menu/base/submenu.h"

// One entry per drawable element; each points the edit level at that element's
// colour and rainbow flag.
class helper_esp_settings_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update_once() override;
    void update() override;
    static helper_esp_settings_menu* get();
};
