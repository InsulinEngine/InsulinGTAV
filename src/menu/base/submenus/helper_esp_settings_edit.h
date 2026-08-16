#pragma once
#include "menu/base/submenu.h"

// Colour and rainbow for one element of one context.
class helper_esp_settings_edit_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update_once() override;
    void update() override;

    // `title` must outlive the menu - a literal from the settings table.
    static void target(color_rgba* colour, bool* rainbow, const char* title);

    static helper_esp_settings_edit_menu* get();
};
