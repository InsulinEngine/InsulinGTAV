#pragma once
#include "menu/base/submenu.h"
#include "menu/base/util/esp.h"

// The shared ESP editor, not a feature. A consumer points it at its context and
// opens it; every level below edits whatever current() returns.
class helper_esp_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update_once() override;
    void update() override;

    // Call before opening. `title` must outlive the menu - a literal.
    static void open_for(menu::esp::esp_context* ctx, const char* title);
    static menu::esp::esp_context* current();

    static helper_esp_menu* get();
};
