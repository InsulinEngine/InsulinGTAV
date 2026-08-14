#include "menu/base/submenus/world_ocean.h"
#include "menu/base/submenus/world.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/control.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    bool  g_disable_waves = false;
    float g_wave_height = 0.f;
}

void world_ocean_menu::load() {
    set_name("Ocean");
    set_parent<world_menu>();

    add_option(toggle_option("Disable Waves")
        .add_toggle(g_disable_waves)
        .add_tooltip("Flattens the sea")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Wave Height")
        .add_number(g_wave_height, "%.1f", 0.5f).add_min(0.f).add_max(10.f)
        .add_savable(get_submenu_name_stack()));
}

void world_ocean_menu::feature_update() {
    // Per-frame overrides, so "off" is simply not pushing them.
    if (g_disable_waves) {
        native::water_override_set_strength(0.f);
    } else if (g_wave_height > 0.f) {
        native::water_override_set_strength(g_wave_height);
    }
}

world_ocean_menu* world_ocean_menu::get() {
    static world_ocean_menu instance;
    return &instance;
}
