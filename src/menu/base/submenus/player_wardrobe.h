#pragma once
#include "menu/base/submenu.h"

// Clothing. Twelve component slots plus the prop slots, each with a drawable and a
// texture index.
//
// The variation counts are per model and per slot and change the moment the model
// does, so the sliders cannot carry a fixed maximum - they are clamped against the
// live count when applied instead. That also makes the menu honest about slots the
// current model does not have: they simply refuse to move off zero.
class player_wardrobe_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static player_wardrobe_menu* get();
};
