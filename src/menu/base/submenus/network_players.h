#pragma once
#include "menu/base/submenu.h"

// Ozark: Network > Players, plus the per-player menu it opens.
//
// The list is rebuilt whenever the player count changes rather than held from
// load, because an entry that outlives its player would act on a freed slot.
class network_players_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void feature_update() override;
    void update_once() override;
    static network_players_menu* get();
};

class network_player_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update_once() override;
    static network_player_menu* get();
};
