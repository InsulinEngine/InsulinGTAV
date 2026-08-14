"""One-shot scaffold for the Vehicle branch, mirroring Ozark's structure.

Not a generator you re-run: it writes the submenu sources once so the whole branch
lands in one consistent shape, after which they are ordinary hand-edited files.
Kept in the repo so the shape is reproducible and reviewable.
"""
import io
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DST = os.path.join(HERE, "..", "src", "menu", "base", "submenus")

HEAD = '''#include "menu/base/submenus/{f}.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {{
    // Everything here acts on the vehicle you are sitting in. Nothing applies on
    // foot, so each entry resolves it first rather than assuming one is there.
    Vehicle my_vehicle() {{
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }}
{state}}}

void {cls}::load() {{
    set_name("{label}");
    set_parent<vehicle_menu>();

{opts}}}

void {cls}::feature_update() {{
{tick}}}

{cls}* {cls}::get() {{
    static {cls} instance;
    return &instance;
}}
'''


def write(f, cls, label, state, opts, tick):
    path = os.path.join(DST, f + ".cpp")
    io.open(path, "w", encoding="utf-8", newline="\n").write(
        HEAD.format(f=f, cls=cls, label=label, state=state, opts=opts, tick=tick))


write("vehicle_health", "vehicle_health_menu", "Health",
      "\n    bool g_auto_repair = false;\n    bool g_auto_wash = false;\n",
      '''    add_option(button_option("Repair Vehicle")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::set_vehicle_fixed(v);
            native::set_vehicle_deformation_fixed(v);
            menu::notify::stacked("Vehicle", "Repaired");
        }));

    add_option(button_option("Wash Vehicle")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_dirt_level(v, 0.f); }));

    add_option(button_option("Dirty Vehicle")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_dirt_level(v, 15.f); }));

    add_option(toggle_option("Auto Repair")
        .add_toggle(g_auto_repair)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Auto Wash")
        .add_toggle(g_auto_wash)
        .add_savable(get_submenu_name_stack()));
''',
      '''    Vehicle v = my_vehicle();
    if (!v)
        return;

    // Only when there is actually damage: SET_VEHICLE_FIXED every frame kills the
    // engine sound and makes the car sit strangely.
    if (g_auto_repair && native::get_vehicle_engine_health(v) < 1000.f) {
        native::set_vehicle_fixed(v);
        native::set_vehicle_deformation_fixed(v);
    }

    if (g_auto_wash && native::get_vehicle_dirt_level(v) > 0.f)
        native::set_vehicle_dirt_level(v, 0.f);
''')

write("vehicle_doors", "vehicle_doors_menu", "Doors",
      "\n    int g_open = 0;\n    int g_close = 0;\n    int g_delete = 0;\n",
      '''    add_option(number_option<int>(SCROLLSELECT, "Open Door")
        .add_number(g_open, "%i", 1).add_min(0).add_max(5)
        .add_tooltip("0 front left, 1 front right, 2 rear left, 3 rear right, 4 hood, 5 boot")
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_door_open(v, g_open, false, false);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Close Door")
        .add_number(g_close, "%i", 1).add_min(0).add_max(5)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_door_shut(v, g_close, false);
        }));

    add_option(number_option<int>(SCROLLSELECT, "Delete Door")
        .add_number(g_delete, "%i", 1).add_min(0).add_max(5)
        .add_update([](number_option<int>*, int) {
            Vehicle v = my_vehicle();
            if (v) native::set_vehicle_door_broken(v, g_delete, false);
        }));

    add_option(button_option("Lock Doors")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_doors_locked(v, 2); }));

    add_option(button_option("Unlock Doors")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_doors_locked(v, 1); }));
''', "")

write("vehicle_gravity", "vehicle_gravity_menu", "Gravity",
      "\n    bool g_slippy = false;\n    bool g_freeze = false;\n    bool g_autoflip = false;\n"
      "    bool g_freeze_latched = false;\n",
      '''    add_option(toggle_option("Drive on Water")
        .add_toggle(g_slippy)
        .add_tooltip("Reduced grip, which is what Ozark drives across water with")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Freeze")
        .add_toggle(g_freeze)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Auto Flip")
        .add_toggle(g_autoflip)
        .add_tooltip("Rights the car when it ends up on its roof")
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Place on Ground")
        .add_click([] { Vehicle v = my_vehicle(); if (v) native::set_vehicle_on_ground_properly(v, 0); }));
''',
      '''    Vehicle v = my_vehicle();
    if (!v)
        return;

    if (g_slippy)
        native::set_vehicle_reduce_grip(v, true);

    // Freeze is the one that needs an explicit release.
    if (g_freeze) {
        native::freeze_entity_position(v, true);
        g_freeze_latched = true;
    } else if (g_freeze_latched) {
        native::freeze_entity_position(v, false);
        g_freeze_latched = false;
    }

    if (g_autoflip && native::is_entity_upsidedown(v))
        native::set_vehicle_on_ground_properly(v, 0);
''')

write("vehicle_collision", "vehicle_collision_menu", "Collision",
      "\n    bool g_no_collision = false;\n    bool g_latched = false;\n",
      '''    add_option(toggle_option("Disable Full Collision")
        .add_toggle(g_no_collision)
        .add_tooltip("Drive through everything")
        .add_savable(get_submenu_name_stack()));
''',
      '''    Vehicle v = my_vehicle();
    if (!v)
        return;

    if (g_no_collision) {
        native::set_entity_collision(v, false, false);
        g_latched = true;
    } else if (g_latched) {
        native::set_entity_collision(v, true, true);
        g_latched = false;
    }
''')

write("vehicle_seats", "vehicle_seats_menu", "Seats",
      "\n    Ped me() { return native::get_player_ped(-1); }\n",
      '''    add_option(button_option("Kick All Seats")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            for (int s = -1; s < 8; s++) {
                Ped p = native::get_ped_in_vehicle_seat(v, s, 0);
                if (p) native::task_leave_vehicle(p, v, 4160);
            }
        }));

    add_option(button_option("Kick All Seats (Exclude Me)")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            for (int s = -1; s < 8; s++) {
                Ped p = native::get_ped_in_vehicle_seat(v, s, 0);
                if (p && p != me()) native::task_leave_vehicle(p, v, 4160);
            }
        }));

    add_option(button_option("Kick Driver")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            Ped p = native::get_ped_in_vehicle_seat(v, -1, 0);
            if (p && p != me()) native::task_leave_vehicle(p, v, 4160);
        }));
''', "")

write("vehicle_autopilot", "vehicle_autopilot_menu", "Autopilot", "",
      '''    add_option(button_option("Enable Autopilot")
        .add_tooltip("Drives to your waypoint, or wanders when none is set")
        .add_click([] {
            Vehicle v = my_vehicle();
            Ped ped = native::get_player_ped(-1);
            if (!v || !ped) return;

            if (native::is_waypoint_active()) {
                Blip b = native::get_first_blip_info_id(8);
                math::vector3<float> c = native::get_blip_info_id_coord(b);
                native::task_vehicle_drive_to_coord_longrange(ped, v, c.x, c.y, c.z, 30.f, 786603, 5.f);
            } else {
                native::task_vehicle_drive_wander(ped, v, 30.f, 786603);
            }
            menu::notify::stacked("Autopilot", "Driving");
        }));

    add_option(button_option("Disable Autopilot")
        .add_click([] {
            native::clear_ped_tasks(native::get_player_ped(-1));
            menu::notify::stacked("Autopilot", "Stopped");
        }));
''', "")

write("vehicle_weapons", "vehicle_weapons_menu", "Weapons",
      "\n    bool g_weapons = false;\n",
      '''    add_option(toggle_option("Toggle Weapons")
        .add_toggle(g_weapons)
        .add_tooltip("Keeps the vehicle weapon enabled")
        .add_savable(get_submenu_name_stack()));
''',
      '''    if (g_weapons && my_vehicle())
        native::set_vehicle_weapons_disabled_this_frame(false);
''')

write("vehicle_particles", "vehicle_particles_menu", "Particle FX",
      "\n    bool g_ptfx = false;\n",
      '''    add_option(toggle_option("Toggle Particle FX")
        .add_toggle(g_ptfx)
        .add_tooltip("Ozark plays a chosen effect on the car. The picker needs the "
                     "PTFX asset lists, which are not ported yet.")
        .add_savable(get_submenu_name_stack()));
''', "")

write("vehicle_speedometer", "vehicle_speedometer_menu", "Speedometer",
      "\n    bool g_plate = false;\n    int g_last_kmh = -1;\n",
      '''    add_option(toggle_option("Numberplate Speedometer")
        .add_toggle(g_plate)
        .add_tooltip("Writes your speed onto the number plate")
        .add_savable(get_submenu_name_stack()));
''',
      '''    Vehicle v = my_vehicle();
    if (!v || !g_plate)
        return;

    int kmh = (int)(native::get_entity_speed(v) * 3.6f);

    // Only on change - rewriting the plate every frame is pointless work.
    if (kmh == g_last_kmh)
        return;
    g_last_kmh = kmh;

    char buf[16];
    snprintf(buf, sizeof(buf), "%d KMH", kmh);
    native::set_vehicle_number_plate_text(v, buf);
''')

write("vehicle_randomization", "vehicle_randomization_menu", "Randomization",
      "\n    bool g_rainbow_primary = false;\n    bool g_rainbow_secondary = false;\n"
      "    int  g_tick = 0;\n",
      '''    add_option(toggle_option("Rainbow Primary Paint")
        .add_toggle(g_rainbow_primary)
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Rainbow Secondary Paint")
        .add_toggle(g_rainbow_secondary)
        .add_savable(get_submenu_name_stack()));
''',
      '''    Vehicle v = my_vehicle();
    if (!v || (!g_rainbow_primary && !g_rainbow_secondary))
        return;

    // Stepping once every few frames: cycling the colour every frame just looks
    // like flicker rather than a colour change.
    if ((++g_tick % 6) != 0)
        return;

    int c = (g_tick / 6) % 160;
    int p = 0, s = 0;
    native::get_vehicle_colours(v, &p, &s);
    if (g_rainbow_primary)   p = c;
    if (g_rainbow_secondary) s = c;
    native::set_vehicle_colours(v, p, s);
''')

write("vehicle_ramps", "vehicle_ramps_menu", "Ramps",
      "\n    bool g_placeholder = false;\n",
      '''    add_option(toggle_option("Front Ramp")
        .add_toggle(g_placeholder)
        .add_tooltip("Ozark attaches ramp props to the car. Needs the object model "
                     "list and attachment offsets, which are not ported yet.")
        .add_savable(get_submenu_name_stack()));
''', "")

write("vehicle_movement", "vehicle_movement_menu", "Movement",
      "\n    bool g_bypass_max_speed = false;\n",
      '''    add_option(toggle_option("Bypass Max Speed")
        .add_toggle(g_bypass_max_speed)
        .add_tooltip("Lifts the speed limiter while you hold the throttle")
        .add_savable(get_submenu_name_stack()));
''',
      '''    Vehicle v = my_vehicle();
    if (v && g_bypass_max_speed)
        native::set_vehicle_max_speed(v, 0.f);
''')

write("vehicle_tyre_tracks", "vehicle_tyre_tracks_menu", "Tire Tracks",
      "\n    bool g_placeholder = false;\n",
      '''    add_option(toggle_option("Persistent Tracks")
        .add_toggle(g_placeholder)
        .add_tooltip("Ozark draws its own tyre trails as particle effects. Needs the "
                     "PTFX lists, which are not ported yet.")
        .add_savable(get_submenu_name_stack()));
''', "")

write("vehicle_customs", "vehicle_customs_menu", "Customs", "",
      '''    add_option(button_option("Max Performance")
        .add_tooltip("Fits the best engine, brakes, transmission, suspension and a turbo")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::set_vehicle_mod_kit(v, 0);
            for (int slot = 11; slot <= 16; slot++) {
                int n = native::get_num_vehicle_mods(v, slot);
                if (n > 0) native::set_vehicle_mod(v, slot, n - 1, false);
            }
            native::toggle_vehicle_mod(v, 18, true);
            menu::notify::stacked("Customs", "Upgraded");
        }));

    add_option(button_option("Remove All Mods")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            native::set_vehicle_mod_kit(v, 0);
            for (int slot = 0; slot <= 49; slot++)
                native::remove_vehicle_mod(v, slot);
            menu::notify::stacked("Customs", "Stock");
        }));
''', "")

write("vehicle_multipliers", "vehicle_multipliers_menu", "Multipliers",
      "\n    float g_torque = 1.f;\n",
      '''    add_option(number_option<float>(SCROLLSELECT, "Engine Torque")
        .add_number(g_torque, "%.2f", 0.25f)
        .add_min(1.f).add_max(20.f)
        .add_tooltip("Multiplies engine power while you are in the car")
        .add_savable(get_submenu_name_stack()));
''',
      '''    Vehicle v = my_vehicle();
    if (v && g_torque > 1.f)
        native::set_vehicle_engine_torque_multiplier(v, g_torque);
''')

write("vehicle_modifiers", "vehicle_modifiers_menu", "Modifiers", "",
      '''    add_option(button_option("Boost Forward")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            math::vector3<float> f = native::get_entity_forward_vector(v);
            native::apply_force_to_entity(v, 1, f.x * 60.f, f.y * 60.f, 0.f,
                                          0.f, 0.f, 0.f, 0, true, true, true, false, true);
        }));
''', "")

print("Vehicle submenu sources written to %s" % os.path.normpath(DST))
