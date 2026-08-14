#include "menu/base/submenus/vehicle_boost.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    float g_forward = 40.f;
    bool  g_unlimited_ability = false;

    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }
}

void vehicle_boost_menu::load() {
    set_name("Boost");
    set_parent<vehicle_menu>();

    add_option(number_option<float>(SCROLLSELECT, "Boost Forwards")
        .add_number(g_forward, "%.0f", 5.f)
        .add_min(5.f).add_max(200.f)
        .add_tooltip("Strength of the forward shove")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Unlimited Special Ability")
        .add_toggle(g_unlimited_ability)
        .add_savable(get_submenu_name_stack()));
}

void vehicle_boost_menu::feature_update() {
    if (g_unlimited_ability)
        native::special_ability_fill_meter(native::player_id(), true);
}

vehicle_boost_menu* vehicle_boost_menu::get() {
    static vehicle_boost_menu instance;
    return &instance;
}
