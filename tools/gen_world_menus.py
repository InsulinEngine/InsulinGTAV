"""One-shot scaffold for the World and Spawner branches, mirroring Ozark.

Spawner reuses src/game/ped_list.h - the same seven categories Ozark's spawner
uses, and the same ones the player model changer draws from.
"""
import io
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DST = os.path.join(HERE, "..", "src", "menu", "base", "submenus")

HDR = '''#pragma once
#include "menu/base/submenu.h"

// Ozark: {branch} > {label}
class {cls} : public menu::submenu::submenu {{
public:
    void load() override;
{fu}    static {cls}* get();
}};
'''

CPP = '''#include "menu/base/submenus/{f}.h"
#include "menu/base/submenus/{parent_h}.h"
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
{extra_inc}
namespace {{
{state}}}

void {cls}::load() {{
    set_name("{label}");
    set_parent<{parent_cls}>();

{opts}}}
{tickdef}
{cls}* {cls}::get() {{
    static {cls} instance;
    return &instance;
}}
'''


def write(f, cls, label, branch, parent_h, parent_cls, state, opts, tick=None, extra_inc=""):
    fu = "    void feature_update() override;\n" if tick is not None else ""
    io.open(os.path.join(DST, f + ".h"), "w", encoding="utf-8", newline="\n").write(
        HDR.format(branch=branch, label=label, cls=cls, fu=fu))
    tickdef = "" if tick is None else "\nvoid %s::feature_update() {\n%s}\n" % (cls, tick)
    io.open(os.path.join(DST, f + ".cpp"), "w", encoding="utf-8", newline="\n").write(
        CPP.format(f=f, cls=cls, label=label, parent_h=parent_h, parent_cls=parent_cls,
                   state=state, opts=opts, tickdef=tickdef, extra_inc=extra_inc))


# ---------------- World > Weather ----------------
write("world_weather", "world_weather_menu", "Weather", "World", "world", "world_menu",
      '''    const char* const k_weather[] = {
        "EXTRASUNNY", "CLEAR", "CLOUDS", "SMOG", "FOGGY", "OVERCAST",
        "RAIN", "THUNDER", "CLEARING", "NEUTRAL", "SNOW", "BLIZZARD",
        "SNOWLIGHT", "XMAS", "HALLOWEEN",
    };
    constexpr int WEATHER_COUNT = (int)(sizeof(k_weather) / sizeof(k_weather[0]));

    int  g_weather = 0;
    bool g_freeze = false;
    scroll_struct<int> g_list[WEATHER_COUNT];
''',
      '''    for (int i = 0; i < WEATHER_COUNT; i++) {
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
''',
      '''    // The weather cycle keeps running, so holding one means re-asserting it.
    if (g_freeze)
        native::set_weather_type_now_persist(k_weather[g_weather]);
''')

# ---------------- World > Time ----------------
write("world_time", "world_time_menu", "Time", "World", "world", "world_menu",
      '''    int  g_hour = 12;
    int  g_minute = 0;
    bool g_freeze = false;
''',
      '''    add_option(number_option<int>(SCROLLSELECT, "Hour")
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
''',
      '''    if (g_freeze)
        native::set_clock_time(g_hour, g_minute, 0);
''')

# ---------------- World > Clear Area ----------------
write("world_clear_area", "world_clear_area_menu", "Clear Area", "World", "world", "world_menu",
      '''    float g_radius = 200.f;

    math::vector3<float> here() {
        return native::get_entity_coords(native::get_player_ped(-1), true);
    }
''',
      '''    add_option(number_option<float>(SCROLLSELECT, "Radius")
        .add_number(g_radius, "%.0f", 25.f).add_min(25.f).add_max(1000.f)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Clear Peds")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area_of_peds(c.x, c.y, c.z, g_radius, 0);
            menu::notify::stacked("World", "Peds cleared");
        }));

    add_option(button_option("Clear Vehicles")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area_of_vehicles(c.x, c.y, c.z, g_radius, false, false, false, false, false, 0);
            menu::notify::stacked("World", "Vehicles cleared");
        }));

    add_option(button_option("Clear Objects")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area_of_objects(c.x, c.y, c.z, g_radius, 0);
        }));

    add_option(button_option("Clear Everything")
        .add_click([] {
            math::vector3<float> c = here();
            native::clear_area(c.x, c.y, c.z, g_radius, true, false, false, false);
            menu::notify::stacked("World", "Area cleared");
        }));
''')

# ---------------- World > Ocean ----------------
write("world_ocean", "world_ocean_menu", "Ocean", "World", "world", "world_menu",
      '''    bool  g_disable_waves = false;
    float g_wave_height = 0.f;
''',
      '''    add_option(toggle_option("Disable Waves")
        .add_toggle(g_disable_waves)
        .add_tooltip("Flattens the sea")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Wave Height")
        .add_number(g_wave_height, "%.1f", 0.5f).add_min(0.f).add_max(10.f)
        .add_savable(get_submenu_name_stack()));
''',
      '''    // Per-frame overrides, so "off" is simply not pushing them.
    if (g_disable_waves) {
        native::water_override_set_strength(0.f);
    } else if (g_wave_height > 0.f) {
        native::water_override_set_strength(g_wave_height);
    }
''')

# ---------------- World > Game FX ----------------
write("world_game_fx", "world_game_fx_menu", "Game FX", "World", "world", "world_menu",
      '''    bool  g_slow_motion = false;
    float g_time_scale = 0.4f;
    float g_gravity = 9.8f;
''',
      '''    add_option(toggle_option("Slow Motion")
        .add_toggle(g_slow_motion).add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Time Scale")
        .add_number(g_time_scale, "%.2f", 0.05f).add_min(0.05f).add_max(1.f)
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Gravity")
        .add_number(g_gravity, "%.1f", 0.5f).add_min(0.f).add_max(20.f)
        .add_tooltip("9.8 is normal")
        .add_update([](number_option<float>*, int) { native::set_gravity_level(0); }));
''',
      '''    // Time scale persists, hence the explicit hand-back to 1.0.
    native::set_time_scale(g_slow_motion ? g_time_scale : 1.f);
''')

# ---------------- World > Local Entities ----------------
write("world_local_entities", "world_local_entities_menu", "Local Entities", "World", "world", "world_menu",
      '''    bool g_no_peds = false;
    bool g_no_traffic = false;
''',
      '''    add_option(toggle_option("No Pedestrians")
        .add_toggle(g_no_peds).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("No Traffic")
        .add_toggle(g_no_traffic).add_savable(get_submenu_name_stack()));
''',
      '''    // Density multipliers are per-frame by design.
    if (g_no_peds)
        native::set_ped_density_multiplier_this_frame(0.f);

    if (g_no_traffic) {
        native::set_vehicle_density_multiplier_this_frame(0.f);
        native::set_random_vehicle_density_multiplier_this_frame(0.f);
        native::set_parked_vehicle_density_multiplier_this_frame(0.f);
    }
''')

# ---------------- Spawner > Peds ----------------
write("spawner_peds", "spawner_peds_menu", "Peds", "Spawner", "spawner", "spawner_menu",
      '''    int g_index[8] = {};
    scroll_struct<int> g_lists[8][32];
    bool g_armed = false;

    void spawn_ped(uint32_t hash) {
        if (!native::is_model_in_cdimage(hash) || !native::is_model_valid(hash)) {
            menu::notify::stacked("Spawner", "Not available on this build");
            return;
        }
        menu::control::request_model(hash, [](uint32_t loaded) {
            Ped me = native::get_player_ped(-1);
            math::vector3<float> pos = native::get_offset_from_entity_in_world_coords(me, 0.f, 3.f, 0.f);
            Ped p = native::create_ped(4, loaded, pos.x, pos.y, pos.z,
                                       native::get_entity_heading(me), false, false);
            native::set_entity_as_mission_entity(p, true, true);
            if (g_armed)
                native::give_weapon_to_ped(p, native::get_hash_key("WEAPON_CARBINERIFLE"), 9999, false, true);
            native::set_model_as_no_longer_needed(loaded);
            menu::notify::stacked("Spawner", "Spawned");
        });
    }
''',
      '''    add_option(toggle_option("Give Weapon")
        .add_toggle(g_armed)
        .add_tooltip("Hands the spawned ped a rifle")
        .add_savable(get_submenu_name_stack()));

    add_option(break_option("Categories").ref());

    for (int g = 0; g < ped_category_count && g < 8; g++) {
        const ped_category& cat = ped_categories[g];

        int n = cat.count < 32 ? cat.count : 32;
        for (int i = 0; i < n; i++) {
            g_lists[g][i].m_name.set(cat.items[i].name);
            g_lists[g][i].m_result = i;
        }

        int gi = g;   // tiny capture: stl::function caps captures at 64 bytes
        add_option(scroll_option<int>(SCROLLSELECT, cat.name)
            .add_scroll(g_index[g], 0, n - 1, g_lists[g])
            .add_click([gi] {
                const ped_category& sel = ped_categories[gi];
                int i = g_index[gi];
                if (i >= 0 && i < sel.count)
                    spawn_ped(sel.items[i].hash);
            }));
    }
''', None, '#include "game/ped_list.h"\n')

print("World and Spawner submenu sources written to %s" % os.path.normpath(DST))
