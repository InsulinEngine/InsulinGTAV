"""One-shot scaffold for the Teleport and Miscellaneous branches, mirroring Ozark."""
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


SUBJECT = '''    // Move the vehicle when seated: pulling the ped out from under a car leaves
    // the car behind and drops you through the world.
    Entity subject() {
        Ped ped = native::get_player_ped(-1);
        if (ped && native::is_ped_in_any_vehicle(ped, false)) {
            Vehicle v = native::get_vehicle_ped_is_in(ped, false);
            if (v) return v;
        }
        return ped;
    }

    void warp(float x, float y, float z) {
        Entity e = subject();
        if (e) native::set_entity_coords_no_offset(e, x, y, z, false, false, false);
    }
'''

# ---------------- Teleport > Directional ----------------
write("teleport_directional", "teleport_directional_menu", "Directional", "Teleport",
      "teleport", "teleport_menu",
      SUBJECT + '''    float g_distance = 20.f;

    void nudge(float fx, float fy, float fz) {
        Entity e = subject();
        if (!e) return;
        math::vector3<float> c = native::get_entity_coords(e, true);
        math::vector3<float> f = native::get_entity_forward_vector(e);
        // Right is the forward vector turned 90 degrees on z.
        warp(c.x + (f.x * fy + f.y * fx) * g_distance,
             c.y + (f.y * fy - f.x * fx) * g_distance,
             c.z + fz * g_distance);
    }
''',
      '''    add_option(number_option<float>(SCROLLSELECT, "Distance")
        .add_number(g_distance, "%.0f", 5.f).add_min(5.f).add_max(200.f)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Forwards").add_click([] { nudge(0.f,  1.f, 0.f); }));
    add_option(button_option("Backwards").add_click([] { nudge(0.f, -1.f, 0.f); }));
    add_option(button_option("Above").add_click([] { nudge(0.f, 0.f,  1.f); }));
    add_option(button_option("Below").add_click([] { nudge(0.f, 0.f, -1.f); }));
    add_option(button_option("Left").add_click([] { nudge(-1.f, 0.f, 0.f); }));
    add_option(button_option("Right").add_click([] { nudge( 1.f, 0.f, 0.f); }));
''')

# ---------------- Teleport > IPL ----------------
write("teleport_ipl", "teleport_ipl_menu", "IPL", "Teleport", "teleport", "teleport_menu",
      SUBJECT + '''    struct place { const char* name; float x, y, z; };
    const place k_places[] = {
        { "North Yankton",      3360.0f, -4849.0f, 112.6f },
        { "Carrier",            3082.0f, -4717.0f,  15.3f },
        { "Desert UFO",         2490.0f,  3774.0f, 2414.0f },
        { "Fort Zancudo UFO",  -2051.0f, 3237.0f, 1456.0f },
        { "Chiliad UFO",         501.0f, 5603.0f,  797.0f },
        { "Cluckin Bell",       -146.0f, 6161.0f,   31.0f },
        { "Eclipse Tower",      -773.0f,  312.0f,  187.0f },
        { "Maze Bank Roof",      -75.0f, -818.0f,  326.0f },
    };
    constexpr int PLACE_COUNT = (int)(sizeof(k_places) / sizeof(k_places[0]));
''',
      '''    for (int i = 0; i < PLACE_COUNT; i++) {
        int idx = i;   // tiny capture: stl::function caps captures at 64 bytes
        add_option(button_option(k_places[i].name)
            .add_click([idx] {
                warp(k_places[idx].x, k_places[idx].y, k_places[idx].z);
                menu::notify::stacked("Teleport", k_places[idx].name);
            }));
    }
''')

# ---------------- Teleport > Save and Load ----------------
write("teleport_save_load", "teleport_save_load_menu", "Save and Load", "Teleport",
      "teleport", "teleport_menu",
      SUBJECT + '''    bool  g_have = false;
    float g_x = 0.f, g_y = 0.f, g_z = 0.f;
''',
      '''    add_option(button_option("Save Current Position")
        .add_click([] {
            Entity e = subject();
            if (!e) return;
            math::vector3<float> c = native::get_entity_coords(e, true);
            g_x = c.x; g_y = c.y; g_z = c.z; g_have = true;
            menu::notify::stacked("Teleport", "Position saved");
        }));

    add_option(button_option("Load Saved Position")
        .add_requirement([] { return g_have; })
        .add_click([] { warp(g_x, g_y, g_z); }));
''')

# ---------------- Misc > Camera ----------------
write("misc_camera", "misc_camera_menu", "Camera", "Miscellaneous", "misc", "misc_menu",
      '''    bool g_first_person_off = false;
''',
      '''    add_option(toggle_option("Disable First Person")
        .add_toggle(g_first_person_off)
        .add_tooltip("Keeps the game out of the first-person camera")
        .add_savable(get_submenu_name_stack()));
''',
      '''    if (g_first_person_off)
        native::disable_first_person_cam_this_frame();
''')

# ---------------- Misc > Radio ----------------
write("misc_radio", "misc_radio_menu", "Radio", "Miscellaneous", "misc", "misc_menu",
      '''    bool g_mobile_radio = false;
''',
      '''    add_option(toggle_option("Mobile Radio")
        .add_toggle(g_mobile_radio)
        .add_tooltip("Radio keeps playing on foot")
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Skip Radio Track")
        .add_click([] { native::skip_radio_forward(); }));
''',
      '''    if (g_mobile_radio)
        native::set_mobile_radio_enabled_during_gameplay(true);
''')

# ---------------- Misc > Visions ----------------
write("misc_visions", "misc_visions_menu", "Visions", "Miscellaneous", "misc", "misc_menu",
      '''    bool g_thermal = false;
    bool g_night = false;
    bool g_thermal_latched = false;
    bool g_night_latched = false;
''',
      '''    add_option(toggle_option("Thermal Vision")
        .add_toggle(g_thermal).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Night Vision")
        .add_toggle(g_night).add_savable(get_submenu_name_stack()));
''',
      '''    // Both persist, so each is switched rather than pushed every frame.
    if (g_thermal != g_thermal_latched) {
        native::set_seethrough(g_thermal);
        g_thermal_latched = g_thermal;
    }
    if (g_night != g_night_latched) {
        native::set_nightvision(g_night);
        g_night_latched = g_night;
    }
''')

# ---------------- Misc > Disables ----------------
write("misc_disables", "misc_disables_menu", "Disables", "Miscellaneous", "misc", "misc_menu",
      '''    bool g_no_phone = false;
    bool g_no_minimap = false;
    bool g_no_cinematic = false;
    bool g_minimap_latched = false;
''',
      '''    add_option(toggle_option("Disable Phone")
        .add_toggle(g_no_phone).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Minimap")
        .add_toggle(g_no_minimap).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Cinematic Camera")
        .add_toggle(g_no_cinematic).add_savable(get_submenu_name_stack()));
''',
      '''    if (g_no_phone)
        native::disable_control_action(0, 27, true);

    if (g_no_cinematic)
        native::set_cinematic_button_active(false);

    if (g_no_minimap) {
        native::display_radar(false);
        g_minimap_latched = true;
    } else if (g_minimap_latched) {
        native::display_radar(true);
        g_minimap_latched = false;
    }
''')

# ---------------- Misc > Dispatch ----------------
write("misc_dispatch", "misc_dispatch_menu", "Dispatch", "Miscellaneous", "misc", "misc_menu",
      '''    // The dispatch service ids the game uses, in the order Ozark lists them.
    struct svc { const char* name; int id; };
    const svc k_services[] = {
        { "Police Automobile",       1 },
        { "Police Helicopter",       2 },
        { "Police Riders",           4 },
        { "Police Vehicle Request",  5 },
        { "Police Road Block",       6 },
        { "Police Boat",            11 },
        { "Swat Automobile",         3 },
        { "Fire Department",         9 },
        { "Ambulance",              10 },
        { "Gangs",                   7 },
        { "Army Vehicle",           15 },
    };
    constexpr int SERVICE_COUNT = (int)(sizeof(k_services) / sizeof(k_services[0]));

    bool g_disabled[SERVICE_COUNT] = {};
''',
      '''    for (int i = 0; i < SERVICE_COUNT; i++) {
        add_option(toggle_option(k_services[i].name)
            .add_toggle(g_disabled[i])
            .add_savable(get_submenu_name_stack()));
    }
''',
      '''    // ENABLE_DISPATCH_SERVICE is per-frame, so "off" simply means not disabling.
    for (int i = 0; i < SERVICE_COUNT; i++) {
        if (g_disabled[i])
            native::enable_dispatch_service(k_services[i].id, false);
    }
''')

print("Teleport and Misc submenu sources written to %s" % os.path.normpath(DST))
