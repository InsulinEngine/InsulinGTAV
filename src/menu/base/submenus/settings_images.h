#pragma once
#include "menu/base/submenu.h"

// Ozark: Settings > Themes > Menu Images
//
// Lets the user point the menu's header and background at a picture in
// /data/Ozark/images (menu::images::list_sources()). Two independent
// sections - header and background each have their own "None" and their own
// picture list, because menu::images::apply()/is_cached() are keyed per slot.
class settings_images_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static settings_images_menu* get();
};
