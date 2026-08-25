#pragma once
#include "menu/base/submenu.h"

// Settings submenu: entry points to Themes, Streamer Mode and Language. The
// theme controls (Save Theme / Reset to Default / theme picker) live in
// settings_themes_menu now; see settings_themes.h.
class settings_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static settings_menu* get();
};

// Language submenu: "English (Default)" reset + the /data/Ozark/lang/*.json
// languages, applied on click. Rebuilt each time it is entered.
class language_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update_once() override;
    static language_menu* get();
};
