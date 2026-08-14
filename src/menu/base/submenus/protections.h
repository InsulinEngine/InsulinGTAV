#pragma once
#include "menu/base/submenu.h"

// Local anti-grief. Event-level filtering needs a network-event hook, which this
// port does not have yet - what is here works without one.
class protections_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;
    static protections_menu* get();
};
