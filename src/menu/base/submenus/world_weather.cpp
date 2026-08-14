#include "menu/base/submenus/world_weather.h"
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
    const char* const k_weather[] = {
        "EXTRASUNNY", "CLEAR", "CLOUDS", "SMOG", "FOGGY", "OVERCAST",
        "RAIN", "THUNDER", "CLEARING", "NEUTRAL", "SNOW", "BLIZZARD",
        "SNOWLIGHT", "XMAS", "HALLOWEEN",
    };
    constexpr int WEATHER_COUNT = (int)(sizeof(k_weather) / sizeof(k_weather[0]));

    int  g_weather = 0;
    bool g_freeze = false;
    scroll_struct<int> g_list[WEATHER_COUNT];
}

void world_weather_menu::load() {
    set_name("Weather");
    set_parent<world_menu>();

    for (int i = 0; i < WEATHER_COUNT; i++) {
        g_list[i].m_name.set(k_weather[i]);
        g_list[i].m_result = i;
    }

    add_option(scroll_option<int>(SCROLLSELECT, "Weather")
        .add_scroll(g_weather, 0, WEATHER_COUNT - 1, g_list)
        .add_click([] {
            native::set_weather_type_now_persist(k_weather[g_weather]);
            menu::notify::stacked("Weather", k_weather[g_weather]);
        }));

    add_option(toggle_option("Freeze Weather")
        .add_toggle(g_freeze)
        .add_tooltip("Holds the selected weather against the game's own cycle")
        .add_savable(get_submenu_name_stack()));
}

void world_weather_menu::feature_update() {
    // The weather cycle keeps running, so holding one means re-asserting it.
    if (g_freeze)
        native::set_weather_type_now_persist(k_weather[g_weather]);
}

world_weather_menu* world_weather_menu::get() {
    static world_weather_menu instance;
    return &instance;
}
