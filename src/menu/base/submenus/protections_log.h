#pragma once
#include "menu/base/submenu.h"

// The last 32 drained protection reports, newest first.
//
// Tier 1 was verified at a desk with the kernel log open over TCP. Tier 2 is
// verified inside live sessions, where the operator has a controller and no
// terminal - so the log has to be reachable from the pause menu.
class protections_log_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    static protections_log_menu* get();
};
