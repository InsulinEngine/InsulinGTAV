#include "menu/base/submenus/world_time.h"
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
    int  g_hour = 12;
    int  g_minute = 0;
    bool g_freeze = false;
}

void world_time_menu::load() {
    set_name("Time");
    set_parent<world_menu>();

    add_option(number_option<int>(SCROLLSELECT, "Hour")
        .add_number(g_hour, "%i", 1).add_min(0).add_max(23)
        .add_update([](number_option<int>*, int) { native::set_clock_time(g_hour, g_minute, 0); }));

    add_option(number_option<int>(SCROLLSELECT, "Minute")
        .add_number(g_minute, "%i", 5).add_min(0).add_max(59)
        .add_update([](number_option<int>*, int) { native::set_clock_time(g_hour, g_minute, 0); }));

    add_option(toggle_option("Freeze Time")
        .add_toggle(g_freeze).add_savable(get_submenu_name_stack()));

    add_option(break_option("Presets").ref());

    add_option(button_option("Morning")
        .add_click([] { g_hour = 8;  g_minute = 0; native::set_clock_time(8, 0, 0); }));
    add_option(button_option("Noon")
        .add_click([] { g_hour = 12; g_minute = 0; native::set_clock_time(12, 0, 0); }));
    add_option(button_option("Evening")
        .add_click([] { g_hour = 19; g_minute = 0; native::set_clock_time(19, 0, 0); }));
    add_option(button_option("Night")
        .add_click([] { g_hour = 1;  g_minute = 0; native::set_clock_time(1, 0, 0); }));
}

void world_time_menu::feature_update() {
    if (g_freeze)
        native::set_clock_time(g_hour, g_minute, 0);
}

world_time_menu* world_time_menu::get() {
    static world_time_menu instance;
    return &instance;
}
