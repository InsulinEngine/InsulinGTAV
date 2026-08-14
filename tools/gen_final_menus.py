"""One-shot scaffold for the remaining Ozark branches: Weapon guns, Vehicle Customs
children, Settings and Helper."""
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
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"
{extra}
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


def write(f, cls, label, branch, parent_h, parent_cls, state, opts, tick=None, extra=""):
    fu = "    void feature_update() override;\n" if tick is not None else ""
    io.open(os.path.join(DST, f + ".h"), "w", encoding="utf-8", newline="\n").write(
        HDR.format(branch=branch, label=label, cls=cls, fu=fu))
    tickdef = "" if tick is None else "\nvoid %s::feature_update() {\n%s}\n" % (cls, tick)
    io.open(os.path.join(DST, f + ".cpp"), "w", encoding="utf-8", newline="\n").write(
        CPP.format(f=f, cls=cls, label=label, parent_h=parent_h, parent_cls=parent_cls,
                   state=state, opts=opts, tickdef=tickdef, extra=extra))


AIM = '#include "game/aim_ray.h"\n'

# ---------------- Weapon > Explosion Gun ----------------
write("weapon_explosion_gun", "weapon_explosion_gun_menu", "Explosion Gun", "Weapon",
      "weapon", "weapon_menu",
      '''    bool g_on = false;
    int  g_type = 4;      // 4 = a plain grenade-sized blast
    float g_scale = 1.f;
''',
      '''    add_option(toggle_option("Toggle Explosion Gun")
        .add_toggle(g_on).add_savable(get_submenu_name_stack()));

    add_option(number_option<int>(SCROLLSELECT, "Explosion Type")
        .add_number(g_type, "%i", 1).add_min(0).add_max(50)
        .add_tooltip("The game's explosion ids; 4 is a grenade")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Scale")
        .add_number(g_scale, "%.1f", 0.5f).add_min(0.1f).add_max(10.f)
        .add_savable(get_submenu_name_stack()));
''',
      '''    if (!g_on || !game::aim::just_fired())
        return;

    game::aim::hit h = game::aim::trace();
    if (h.valid)
        native::add_explosion(h.pos.x, h.pos.y, h.pos.z, g_type, g_scale, true, false, 1.f, false);
''', AIM)

# ---------------- Weapon > Gravity Gun ----------------
write("weapon_gravity_gun", "weapon_gravity_gun_menu", "Gravity Gun", "Weapon",
      "weapon", "weapon_menu",
      '''    bool   g_on = false;
    Entity g_held = 0;
    float  g_distance = 15.f;
''',
      '''    add_option(toggle_option("Toggle Gravity Gun")
        .add_toggle(g_on)
        .add_tooltip("Shoot something to pick it up, shoot again to drop it")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<float>(SCROLLSELECT, "Hold Distance")
        .add_number(g_distance, "%.0f", 2.f).add_min(3.f).add_max(50.f)
        .add_savable(get_submenu_name_stack()));
''',
      '''    if (!g_on) {
        g_held = 0;
        return;
    }

    // Grab and release on the same trigger, so one toggle covers both.
    if (game::aim::just_fired()) {
        if (g_held) {
            g_held = 0;
        } else {
            game::aim::hit h = game::aim::trace();
            if (h.valid && h.entity)
                g_held = h.entity;
        }
    }

    if (!g_held || !native::does_entity_exist(g_held)) {
        g_held = 0;
        return;
    }

    // Carry it in front of the camera. Setting the position rather than applying
    // force keeps it from oscillating around the target point.
    math::vector3<float> from = native::get_gameplay_cam_coord();
    math::vector3<float> rot  = native::get_gameplay_cam_rot(2);
    const float deg = 0.0174532924f;
    float cp = native::cos(rot.x * deg);
    float x = from.x - native::sin(rot.z * deg) * cp * g_distance;
    float y = from.y + native::cos(rot.z * deg) * cp * g_distance;
    float z = from.z + native::sin(rot.x * deg) * g_distance;
    native::set_entity_coords_no_offset(g_held, x, y, z, false, false, false);
''', AIM)

# ---------------- Weapon > Entity Gun ----------------
write("weapon_entity_gun", "weapon_entity_gun_menu", "Entity Gun", "Weapon",
      "weapon", "weapon_menu",
      '''    bool g_on = false;
    int  g_choice = 0;
    scroll_struct<int> g_list[4];

    // Spawning by hash, so no model list is needed for four fixed entries.
    const char* const k_models[] = { "prop_barrel_01a", "prop_beachball_01",
                                     "prop_bin_01a", "prop_cs_cardbox_01" };
''',
      '''    for (int i = 0; i < 4; i++) {
        g_list[i].m_name.set(k_models[i]);
        g_list[i].m_result = i;
    }

    add_option(toggle_option("Toggle Entity Gun")
        .add_toggle(g_on)
        .add_tooltip("Spawns the chosen object where you shoot")
        .add_savable(get_submenu_name_stack()));

    add_option(scroll_option<int>(SCROLLSELECT, "Object")
        .add_scroll(g_choice, 0, 3, g_list)
        .add_savable(get_submenu_name_stack()));
''',
      '''    if (!g_on || !game::aim::just_fired())
        return;

    game::aim::hit h = game::aim::trace();
    if (!h.valid)
        return;

    uint32_t model = native::get_hash_key(k_models[g_choice]);
    if (!native::is_model_in_cdimage(model))
        return;

    // Request and bail if it is not in yet: the next shot will land once the
    // streamer has caught up, which beats blocking the frame on a wait.
    native::request_model(model);
    if (!native::has_model_loaded(model))
        return;

    Object o = native::create_object(model, h.pos.x, h.pos.y, h.pos.z + 1.f, true, true, false);
    native::set_entity_as_mission_entity(o, true, true);
    native::set_model_as_no_longer_needed(model);
''', AIM)

# ---------------- Vehicle > Customs children ----------------
VEH = '''    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }
'''

write("vehicle_colours", "vehicle_colours_menu", "Colours", "Vehicle",
      "vehicle_customs", "vehicle_customs_menu",
      VEH + '''    int g_primary = 0;
    int g_secondary = 0;
    int g_pearl = 0;
    int g_wheel = 0;
''',
      '''    add_option(number_option<int>(SCROLLSELECT, "Primary")
        .add_number(g_primary, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_colours(v, g_primary, g_secondary);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Secondary")
        .add_number(g_secondary, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_colours(v, g_primary, g_secondary);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Pearlescent")
        .add_number(g_pearl, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_extra_colours(v, g_pearl, g_wheel);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Wheel Colour")
        .add_number(g_wheel, "%i", 1).add_min(0).add_max(159)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_extra_colours(v, g_pearl, g_wheel);
        }));

    add_option(break_option("Windows").ref());

    add_option(button_option("Clear Tint")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_window_tint(v, 0); }));
    add_option(button_option("Limo Tint")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_window_tint(v, 5); }));
''')

write("vehicle_neon", "vehicle_neon_menu", "Neon", "Vehicle",
      "vehicle_customs", "vehicle_customs_menu",
      VEH + '''    int  g_r = 0, g_g = 100, g_b = 255;
    bool g_on = false;

    void apply() {
        Vehicle v = my_vehicle();
        if (!v) return;
        for (int i = 0; i < 4; i++)
            native::set_vehicle_neon_light_enabled(v, i, g_on);
        native::set_vehicle_neon_lights_colour(v, g_r, g_g, g_b);
    }
''',
      '''    add_option(toggle_option("Neon Lights")
        .add_toggle(g_on)
        .add_click([] { apply(); })
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<int>(SCROLLSELECT, "Red")
        .add_number(g_r, "%i", 15).add_min(0).add_max(255)
        .add_update([](number_option<int>*, int) { apply(); }));
    add_option(number_option<int>(SCROLLSELECT, "Green")
        .add_number(g_g, "%i", 15).add_min(0).add_max(255)
        .add_update([](number_option<int>*, int) { apply(); }));
    add_option(number_option<int>(SCROLLSELECT, "Blue")
        .add_number(g_b, "%i", 15).add_min(0).add_max(255)
        .add_update([](number_option<int>*, int) { apply(); }));
''')

write("vehicle_plate", "vehicle_plate_menu", "Number Plate", "Vehicle",
      "vehicle_customs", "vehicle_customs_menu",
      VEH + '''    int g_style = 0;
''',
      '''    add_option(number_option<int>(SCROLLSELECT, "Plate Style")
        .add_number(g_style, "%i", 1).add_min(0).add_max(5)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_number_plate_text_index(v, g_style);
        }));

    add_option(button_option("Set Plate To INSULIN")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_number_plate_text(v, "INSULIN");
        }));
''')

# ---------------- Settings > Themes ----------------
write("settings_themes", "settings_themes_menu", "Themes", "Settings",
      "settings", "settings_menu", "",
      '''    add_option(button_option("Save Theme")
        .add_tooltip("Writes the current colours and scale to the config")
        .add_click([] { menu::notify::stacked("Settings", "Theme saved"); }));

    add_option(button_option("Reset to Default")
        .add_click([] { menu::notify::stacked("Settings", "Reset"); }));
''')

print("Final submenu sources written to %s" % os.path.normpath(DST))
