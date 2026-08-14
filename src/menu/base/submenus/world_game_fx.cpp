#include "menu/base/submenus/world_game_fx.h"
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
    bool  g_slow_motion = false;
    float g_time_scale = 0.4f;
    float g_gravity = 9.8f;
}

void world_game_fx_menu::load() {
    set_name("Game FX");
    set_parent<world_menu>();

    add_option(toggle_option("Slow Motion")
        .add_toggle(g_slow_motion).add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Time Scale")
        .add_number(g_time_scale, "%.2f", 0.05f).add_min(0.05f).add_max(1.f)
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Gravity")
        .add_number(g_gravity, "%.1f", 0.5f).add_min(0.f).add_max(20.f)
        .add_tooltip("9.8 is normal")
        .add_update([](number_option<float>*, int) { native::set_gravity_level(0); }));
}

void world_game_fx_menu::feature_update() {
    // Time scale persists, hence the explicit hand-back to 1.0.
    native::set_time_scale(g_slow_motion ? g_time_scale : 1.f);
}

world_game_fx_menu* world_game_fx_menu::get() {
    static world_game_fx_menu instance;
    return &instance;
}
