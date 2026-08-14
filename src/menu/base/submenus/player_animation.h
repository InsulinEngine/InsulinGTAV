#pragma once
#include "menu/base/submenu.h"

// Scenarios and walk styles - Ozark's Player > Animation, minus the emote
// browser, which needs an anim-dictionary list this port does not carry yet.
class player_animation_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_animation_menu* get();
};
