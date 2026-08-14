#pragma once
#include "menu/base/submenu.h"

// First real feature submenu (Story Mode): validates the feature-authoring
// workflow on the ported base. Options call natives directly; the persistent
// toggles are re-applied every frame from feature_update().
class player_menu : public menu::submenu::submenu {
public:
    static player_menu* get();

    void load() override;
    void feature_update() override;

    player_menu() : menu::submenu::submenu() {}
};
