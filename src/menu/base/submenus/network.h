#pragma once
#include "menu/base/submenu.h"

// Session state. Most of this is inert in Story Mode and only means something
// once a session is up.
class network_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static network_menu* get();
};
