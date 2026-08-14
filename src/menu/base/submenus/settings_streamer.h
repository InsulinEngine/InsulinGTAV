#pragma once
#include "menu/base/submenu.h"

// Ozark: Settings > Streamer Mode
class settings_streamer_menu : public menu::submenu::submenu {
public:
    void load() override;
    static settings_streamer_menu* get();
};
