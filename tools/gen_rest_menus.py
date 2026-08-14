"""One-shot scaffold for the remaining Vehicle, World and Settings sub-branches."""
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


def write(f, cls, label, branch, parent_h, parent_cls, state, opts, tick=None):
    fu = "    void feature_update() override;\n" if tick is not None else ""
    io.open(os.path.join(DST, f + ".h"), "w", encoding="utf-8", newline="\n").write(
        HDR.format(branch=branch, label=label, cls=cls, fu=fu))
    tickdef = "" if tick is None else "\nvoid %s::feature_update() {\n%s}\n" % (cls, tick)
    io.open(os.path.join(DST, f + ".cpp"), "w", encoding="utf-8", newline="\n").write(
        CPP.format(f=f, cls=cls, label=label, parent_h=parent_h, parent_cls=parent_cls,
                   state=state, opts=opts, tickdef=tickdef))


VEH = '''    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }
'''

# ---------------- Vehicle > Movement > Acrobatics ----------------
write("vehicle_acrobatics", "vehicle_acrobatics_menu", "Acrobatics", "Vehicle",
      "vehicle_movement", "vehicle_movement_menu",
      VEH + '''    float g_force = 20.f;

    // Rotation impulses are applied about the vehicle's own axes, which is what
    // makes them read as a roll or a flip rather than a shove in world space.
    void spin(float rx, float ry, float rz) {
        Vehicle v = my_vehicle();
        if (!v) return;
        native::apply_force_to_entity(v, 1, 0.f, 0.f, 0.f,
                                      rx * g_force, ry * g_force, rz * g_force,
                                      0, true, true, true, false, true);
    }
''',
      '''    add_option(number_option<float>(SCROLLSELECT, "Force")
        .add_number(g_force, "%.0f", 5.f).add_min(5.f).add_max(100.f)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Barrel Roll").add_click([] { spin(0.f, 1.f, 0.f); }));
    add_option(button_option("Front Flip").add_click([] { spin(1.f, 0.f, 0.f); }));
    add_option(button_option("Back Flip").add_click([] { spin(-1.f, 0.f, 0.f); }));
    add_option(button_option("Spin").add_click([] { spin(0.f, 0.f, 1.f); }));

    add_option(button_option("Launch Up")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::apply_force_to_entity(v, 1, 0.f, 0.f, g_force * 3.f,
                                          0.f, 0.f, 0.f, 0, true, true, true, false, true);
        }));
''')

# ---------------- Vehicle > Movement > Parachute ----------------
write("vehicle_parachute", "vehicle_parachute_menu", "Parachute", "Vehicle",
      "vehicle_movement", "vehicle_movement_menu",
      VEH + '''    bool g_auto_deploy = false;
''',
      '''    add_option(toggle_option("Auto Deploy")
        .add_toggle(g_auto_deploy)
        .add_tooltip("Opens the vehicle parachute once you are falling")
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Deploy Now")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_parachute_active(v, true);
        }));
''',
      '''    Vehicle v = my_vehicle();
    if (!v || !g_auto_deploy)
        return;

    // Only once actually falling: deploying on the ground does nothing and
    // re-arming it every frame would stop it ever opening.
    if (native::is_entity_in_air(v))
        native::set_vehicle_parachute_active(v, true);
''')

# ---------------- World > Bullet Tracers ----------------
write("world_bullet_tracers", "world_bullet_tracers_menu", "Bullet Tracers", "World",
      "world", "world_menu",
      '''    bool g_tracers = false;
    int  g_r = 255, g_g = 0, g_b = 0;
''',
      '''    add_option(toggle_option("Draw Tracers")
        .add_toggle(g_tracers)
        .add_tooltip("Draws a line along your shots")
        .add_savable(get_submenu_name_stack()));

    add_option(number_option<int>(SCROLLSELECT, "Red")
        .add_number(g_r, "%i", 15).add_min(0).add_max(255)
        .add_savable(get_submenu_name_stack()));
    add_option(number_option<int>(SCROLLSELECT, "Green")
        .add_number(g_g, "%i", 15).add_min(0).add_max(255)
        .add_savable(get_submenu_name_stack()));
    add_option(number_option<int>(SCROLLSELECT, "Blue")
        .add_number(g_b, "%i", 15).add_min(0).add_max(255)
        .add_savable(get_submenu_name_stack()));
''',
      '''    if (!g_tracers)
        return;

    Ped ped = native::get_player_ped(-1);
    if (!ped || !native::is_ped_shooting(ped))
        return;

    // Muzzle to impact. GET_PED_LAST_WEAPON_IMPACT_COORD only returns something on
    // the frames a shot actually lands, which is what keeps this from drawing a
    // stale line for the whole burst.
    math::vector3<float> impact = { 0.f, 0.f, 0.f };
    if (!native::get_ped_last_weapon_impact_coord(ped, &impact))
        return;

    math::vector3<float> from = native::get_ped_bone_coords(ped, 6286, 0.f, 0.f, 0.f);
    native::draw_line(from.x, from.y, from.z, impact.x, impact.y, impact.z, g_r, g_g, g_b, 200);
''')

# ---------------- World > Trains ----------------
write("world_trains", "world_trains_menu", "Train", "World", "world", "world_menu",
      '''    bool g_no_trains = false;
''',
      '''    add_option(toggle_option("Disable Trains")
        .add_toggle(g_no_trains)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Delete All Trains")
        .add_click([] {
            native::delete_all_trains();
            menu::notify::stacked("World", "Trains removed");
        }));
''',
      '''    // SET_RANDOM_TRAINS is a switch, not a per-frame push, but re-asserting it is
    // harmless and survives the game turning them back on after a load.
    if (g_no_trains)
        native::set_random_trains(false);
''')

# ---------------- Settings > Streamer Mode ----------------
write("settings_streamer", "settings_streamer_menu", "Streamer Mode", "Settings",
      "settings", "settings_menu",
      '''    bool g_hide_names = false;
''',
      '''    add_option(toggle_option("Hide Player Names")
        .add_toggle(g_hide_names)
        .add_tooltip("Session only - there are no other players in Story Mode")
        .add_savable(get_submenu_name_stack()));
''')

print("Remaining submenu sources written to %s" % os.path.normpath(DST))
