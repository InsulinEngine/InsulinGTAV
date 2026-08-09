#include "menu/base/submenus/main.h"

// M1: empty root submenu. M2 will populate load() with a demo of every option
// type.
void main_menu::load() {}
void main_menu::update_once() {}
void main_menu::update() {}
void main_menu::feature_update() {}

main_menu* main_menu::get() {
    static main_menu instance;
    return &instance;
}
