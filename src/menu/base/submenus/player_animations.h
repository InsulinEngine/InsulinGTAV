#pragma once
#include "menu/base/submenu.h"

// Ozark: Player > Animation > Animations. Emotes from anim dictionaries, which have
// to be streamed before they play.
class player_animations_menu : public menu::submenu::submenu {
public:
    void load() override;
    static player_animations_menu* get();
};
