#pragma once
#include "menu/base/submenu.h"

// Demo root submenu + one child, exercising the option types ported in M2
// (button / toggle / break / submenu_option). Fully navigable.
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

class demo_child : public menu::submenu::submenu {
public:
    static demo_child* get();

    void load() override;

    demo_child()
        : menu::submenu::submenu()
    {}
};
