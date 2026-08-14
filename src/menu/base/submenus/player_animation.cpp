#include "menu/base/submenus/player_animation.h"
#include "menu/base/submenus/player.h"
#include "menu/base/submenus/player_animations.h"
#include "menu/base/submenus/player_scenarios.h"
#include "menu/base/submenus/player_clipsets.h"
#include "menu/base/options/submenu_option.h"

void player_animation_menu::load() {
    set_name("Animation");
    set_parent<player_menu>();

    add_option(submenu_option("Animations").add_submenu<player_animations_menu>());
    add_option(submenu_option("Scenarios").add_submenu<player_scenarios_menu>());
    add_option(submenu_option("Clipsets").add_submenu<player_clipsets_menu>());
}

player_animation_menu* player_animation_menu::get() {
    static player_animation_menu instance;
    return &instance;
}
