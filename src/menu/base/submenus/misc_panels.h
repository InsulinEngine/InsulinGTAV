#pragma once
#include "menu/base/submenu.h"

class misc_panels_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static misc_panels_menu* get();
};
