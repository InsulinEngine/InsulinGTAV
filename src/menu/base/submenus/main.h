#pragma once
#include "menu/base/submenu.h"

// Demo root submenu. For M1 it is empty (the menu opens and renders the frame +
// header). M2 fills load() with one of each option type.
class main_menu : public menu::submenu::submenu {
public:
    static main_menu* get();

    void load() override;
    void update_once() override;
    void update() override;
    void feature_update() override;

    main_menu()
        : menu::submenu::submenu()
    {}
};
