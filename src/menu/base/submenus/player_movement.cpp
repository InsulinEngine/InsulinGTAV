#include "menu/base/submenus/player_movement.h"
#include "menu/base/submenus/player.h"
#include "menu/base/options/toggle.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"   // get_entity_velocity

namespace {
    bool g_super_jump = false;
    bool g_walk_on_air = false;

    Ped self_ped() { return native::get_player_ped(-1); }
    Player self_player() { return native::player_id(); }
}

void player_movement_menu::load() {
    set_name("Movement");
    set_parent<player_menu>();

    add_option(toggle_option("Super Jump")
        .add_toggle(g_super_jump)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Walk on Air")
        .add_toggle(g_walk_on_air)
        .add_tooltip("Holds you at the height you switched it on at")
        .add_savable(get_submenu_name_stack()));
}

void player_movement_menu::feature_update() {
    if (g_super_jump)
        native::set_super_jump_this_frame(self_player());

    // Freezing the height each frame is what "walking on air" is: the ped keeps
    // its own x/y movement and only the fall is cancelled.
    if (g_walk_on_air) {
        Ped ped = self_ped();
        if (ped && !native::is_ped_in_any_vehicle(ped, false)) {
            math::vector3<float> v = native::get_entity_velocity(ped);
            native::set_entity_velocity(ped, v.x, v.y, 0.f);
        }
    }
}

player_movement_menu* player_movement_menu::get() {
    static player_movement_menu instance;
    return &instance;
}
