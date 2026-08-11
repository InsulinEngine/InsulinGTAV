#pragma once
#include "menu/base/submenu.h"

// Settings submenu: theme controls. "Save Theme" (on-screen keyboard) + "Reset to
// Default" are fixed; the saved themes in /data/insulin/themes are listed below
// and applied on click. Rebuilt each time the submenu is entered (update_once).
class settings_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static settings_menu* get();
};

// Language submenu: "English (Default)" reset + the /data/insulin/lang/*.json
// languages, applied on click. Rebuilt each time it is entered.
class language_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update_once() override;
    static language_menu* get();
};
