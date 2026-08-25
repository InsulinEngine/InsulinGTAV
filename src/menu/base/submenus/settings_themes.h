#pragma once
#include "menu/base/submenu.h"

// Ozark: Settings > Themes
class settings_themes_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static settings_themes_menu* get();

    // Reads the "LastTheme" key under this submenu's own name stack and, if
    // non-empty, applies that theme by name. No-ops on an empty key or a
    // missing file. Must run after load() has set this submenu's name stack
    // (get_submenu_name_stack() is meaningless before that) and after
    // util::config::load() (the key has to already be in memory).
    static void apply_last_theme();
};
