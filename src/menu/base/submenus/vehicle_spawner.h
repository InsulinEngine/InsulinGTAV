#pragma once
#include "menu/base/submenu.h"

// Vehicle spawner. The class list is fixed (23 GTA V vehicle classes); the vehicle
// list per class is rebuilt on entry from src/game/vehicle_list.h, which is generated
// from the game's own vehicles.meta set (872 vehicles, base game + every DLC).
class vehicle_spawner_menu : public menu::submenu::submenu {
public:
    void load() override;
    static vehicle_spawner_menu* get();
};

// One reused submenu for whichever class was picked. Rebuilt whenever the selected
// class changes, the same dirty-flag pattern settings_menu uses for themes.
class vehicle_class_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static vehicle_class_menu* get();
};
