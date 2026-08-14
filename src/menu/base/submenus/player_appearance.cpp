#include "menu/base/submenus/player_appearance.h"
#include "menu/base/submenus/player.h"
#include "menu/base/submenus/player_model.h"
#include "menu/base/submenus/player_wardrobe.h"
#include "menu/base/options/submenu_option.h"

// Exactly Ozark: this menu is only a fork to Model and Wardrobe.
void player_appearance_menu::load() {
    set_name("Appearance");
    set_parent<player_menu>();

    add_option(submenu_option("Model").add_submenu<player_model_menu>());
    add_option(submenu_option("Wardrobe").add_submenu<player_wardrobe_menu>());
}

player_appearance_menu* player_appearance_menu::get() {
    static player_appearance_menu instance;
    return &instance;
}
