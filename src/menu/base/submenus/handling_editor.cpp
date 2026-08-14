#include "menu/base/submenus/handling_editor.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/options/number.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/invoker.h"
#include "platform/log.h"

namespace {
    constexpr float PI_F = 3.14159265358979f;

    // ---- eboot RVAs / offsets (CUSA00411 v1.57, imagebase 0) -----------------
    constexpr uint64_t RVA_GUID_TO_BASE = 0x1EE5180;  // fwScriptGuid::GetBaseFromGuid
    constexpr uint32_t VEH_HANDLING_OFF = 0x930;      // CVehicle::pHandling
    constexpr uint32_t VEH_GRAVITY_OFF  = 0xC6C;      // what SET_VEHICLE_GRAVITY writes

    typedef void* (*guid_to_base_fn)(uint32_t guid);

    // how the shown value maps onto the value in memory
    enum field_kind : uint8_t {
        FK_RAW,      // 1:1
        FK_DRAG,     // meta = memory * 10000
        FK_KMH,      // meta km/h = memory m/s * 3.6
        FK_STEER,    // degrees; also writes the reciprocal at +0x84
        FK_TMAX,     // degrees-free; writes 1/x at +0x8C and 1/(max-min) at +0x94
        FK_TMIN,     // writes 1/(max-min) at +0x94
        FK_TLAT,     // degrees; writes 1/rad at +0x9C
        FK_GRAV,     // lives on CVehicle, not CHandlingData
    };

    struct hfield {
        const char* name;
        uint16_t    off;
        float       step;
        float       lo;
        float       hi;
        uint8_t     kind;
        const char* tip;
    };

    // Offsets verified live against handling.meta on this build.
    const hfield g_fields[] = {
        { "Mass (kg)",            0x0C,  25.f,     1.f, 100000.f, FK_RAW,  "fMass" },
        { "Drag",                 0x10,  0.5f,     0.f,   1000.f, FK_DRAG, "fInitialDragCoeff, meta units" },
        { "Acceleration",         0x60,  0.01f,    0.f,     10.f, FK_RAW,  "fInitialDriveForce" },
        { "Top Speed (km/h)",     0x68,  5.f,      1.f,   1000.f, FK_KMH,  "fInitialDriveMaxFlatVel" },
        { "Drive Inertia",        0x54,  0.05f,    0.f,     10.f, FK_RAW,  "fDriveInertia" },
        { "Brake Force",          0x6C,  0.05f,    0.f,     20.f, FK_RAW,  "fBrakeForce" },
        { "Handbrake Force",      0x7C,  0.05f,    0.f,     20.f, FK_RAW,  "fHandBrakeForce" },
        { "Steering Lock (deg)",  0x80,  1.f,      1.f,     90.f, FK_STEER,"fSteeringLock, writes the reciprocal too" },
        { "Traction Max",         0x88,  0.05f,    0.f,     50.f, FK_TMAX, "fTractionCurveMax + its derived twins" },
        { "Traction Min",         0x90,  0.05f,    0.f,     50.f, FK_TMIN, "fTractionCurveMin + derived" },
        { "Traction Lateral (deg)",0x98, 0.5f,     1.f,     90.f, FK_TLAT, "fTractionCurveLateral" },
        { "Traction Loss Mult",   0xB8,  0.05f,    0.f,     10.f, FK_RAW,  "fTractionLossMult" },
        { "Suspension Force",     0xBC,  0.1f,     0.f,    100.f, FK_RAW,  "fSuspensionForce" },
        { "Suspension Raise",     0xD0,  0.01f,   -1.f,      1.f, FK_RAW,  "fSuspensionRaise" },
        { "Anti-Roll Bar",        0xDC,  0.1f,     0.f,    100.f, FK_RAW,  "fAntiRollBarForce" },
        { "Collision Damage",     0xF0,  0.05f,    0.f,     10.f, FK_RAW,  "fCollisionDamageMult" },
        { "Deformation Damage",   0xF8,  0.05f,    0.f,     10.f, FK_RAW,  "fDeformationDamageMult" },
        { "Engine Damage",        0xFC,  0.05f,    0.f,     10.f, FK_RAW,  "fEngineDamageMult" },
        { "Rocket Boost Cap",     0x11C, 0.25f,    0.f,    100.f, FK_RAW,  "fRocketBoostCapacity" },
        { "Boost Max Speed",      0x120, 5.f,      0.f,   1000.f, FK_RAW,  "fBoostMaxSpeed" },
        { "Gravity",              VEH_GRAVITY_OFF, 0.5f, -100.f, 100.f, FK_GRAV, "CVehicle gravity, what SET_VEHICLE_GRAVITY writes" },
    };
    constexpr int FIELD_COUNT = (int)(sizeof(g_fields) / sizeof(g_fields[0]));

    float     g_val[FIELD_COUNT]  = {};   // what the options edit
    float     g_prev[FIELD_COUNT] = {};   // last value pushed to memory
    float     g_orig[FIELD_COUNT] = {};   // values as first seen, for Restore
    uintptr_t g_bound = 0;                // handling pointer the mirrors belong to
    bool      g_have_orig = false;

    uintptr_t vehicle_ptr() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        Vehicle veh = native::get_vehicle_ped_is_in(ped, false);
        if (!veh || !rage::invoker::g_eboot_base)
            return 0;
        guid_to_base_fn to_base = (guid_to_base_fn)(rage::invoker::g_eboot_base + RVA_GUID_TO_BASE);
        return (uintptr_t)to_base((uint32_t)veh);
    }

    uintptr_t handling_ptr() {
        uintptr_t veh = vehicle_ptr();
        if (!veh)
            return 0;
        return *(uintptr_t*)(veh + VEH_HANDLING_OFF);
    }

    // base pointer a field lives on: gravity is on the vehicle, the rest on handling
    uintptr_t base_for(const hfield& f) {
        return f.kind == FK_GRAV ? vehicle_ptr() : handling_ptr();
    }

    float to_ui(const hfield& f, float mem) {
        switch (f.kind) {
        case FK_DRAG:  return mem * 10000.f;
        case FK_KMH:   return mem * 3.6f;
        case FK_STEER:
        case FK_TLAT:  return mem * (180.f / PI_F);
        default:       return mem;
        }
    }

    float to_mem(const hfield& f, float ui) {
        switch (f.kind) {
        case FK_DRAG:  return ui / 10000.f;
        case FK_KMH:   return ui / 3.6f;
        case FK_STEER:
        case FK_TLAT:  return ui * (PI_F / 180.f);
        default:       return ui;
        }
    }

    // Write one field plus everything the game derives from it. Skipping the derived
    // values is the classic bug: the grip curve stops matching its own reciprocals.
    void write_field(uintptr_t handling, const hfield& f, float ui) {
        uintptr_t base = base_for(f);
        if (!base)
            return;

        float mem = to_mem(f, ui);
        *(float*)(base + f.off) = mem;

        if (!handling)
            return;

        switch (f.kind) {
        case FK_STEER:
            *(float*)(handling + 0x84) = mem != 0.f ? 1.f / mem : 0.f;
            break;
        case FK_TMAX: {
            *(float*)(handling + 0x8C) = mem != 0.f ? 1.f / mem : 0.f;
            float mn = *(float*)(handling + 0x90);
            float d = mem - mn;
            *(float*)(handling + 0x94) = d != 0.f ? 1.f / d : 0.f;
            break;
        }
        case FK_TMIN: {
            float mx = *(float*)(handling + 0x88);
            float d = mx - mem;
            *(float*)(handling + 0x94) = d != 0.f ? 1.f / d : 0.f;
            break;
        }
        case FK_TLAT:
            *(float*)(handling + 0x9C) = mem != 0.f ? 1.f / mem : 0.f;
            break;
        default:
            break;
        }
    }

    // Pull the live values into the mirrors; remember the first set for Restore.
    void read_all() {
        uintptr_t handling = handling_ptr();
        g_bound = handling;
        if (!handling)
            return;

        for (int i = 0; i < FIELD_COUNT; i++) {
            uintptr_t base = base_for(g_fields[i]);
            float mem = base ? *(float*)(base + g_fields[i].off) : 0.f;
            g_val[i] = to_ui(g_fields[i], mem);
            g_prev[i] = g_val[i];
        }
        if (!g_have_orig) {
            for (int i = 0; i < FIELD_COUNT; i++) g_orig[i] = g_val[i];
            g_have_orig = true;
        }
    }
}

void handling_editor_menu::load() {
    set_name("Handling Editor");
    set_parent<vehicle_menu>();

    add_option(button_option("Restore Original")
        .add_tooltip("Put back the values this vehicle had when you first opened the editor")
        .add_click([] {
            uintptr_t handling = handling_ptr();
            if (!handling || !g_have_orig) {
                menu::notify::stacked("Handling", "Nothing to restore");
                return;
            }
            for (int i = 0; i < FIELD_COUNT; i++) {
                g_val[i] = g_orig[i];
                g_prev[i] = g_orig[i];
                write_field(handling, g_fields[i], g_orig[i]);
            }
            menu::notify::stacked("Handling", "Restored");
        }));


    add_option(break_option("Values (shared by every vehicle of this model)").ref());

    for (int i = 0; i < FIELD_COUNT; i++) {
        int idx = i;    // tiny capture: stl::function caps captures at 64 bytes
        add_option(number_option<float>(SCROLLSELECT, g_fields[i].name)
            .add_number(g_val[i], "%.3f", g_fields[i].step)
            .add_min(g_fields[i].lo)
            .add_max(g_fields[i].hi)
            .add_tooltip(g_fields[i].tip)
            .add_requirement([] { return handling_ptr() != 0; })
            .add_update([idx](number_option<float>*, int) {
                // Only write once the mirrors hold values actually read from THIS
                // handling block. g_bound is set by read_all() alone, so an
                // unbound editor stays read-only. Without that check the option's
                // own min-clamp is enough to push a value: g_val starts at 0, a
                // field whose minimum is above 0 gets clamped up on construction,
                // the update fires, sees a change against g_prev and writes a made
                // up number into a car nobody asked it to touch.
                uintptr_t handling = handling_ptr();
                if (handling && handling == g_bound && g_val[idx] != g_prev[idx]) {
                    write_field(handling, g_fields[idx], g_val[idx]);
                    g_prev[idx] = g_val[idx];
                }
            }));
    }
}

void handling_editor_menu::update() {
    // Re-read whenever the player changes vehicle, so the sliders always show the
    // handling that is actually in front of you.
    uintptr_t handling = handling_ptr();
    if (handling != g_bound) {
        g_have_orig = false;
        read_all();
    }
}

void handling_editor_menu::update_once() {
    g_have_orig = false;
    read_all();
}

handling_editor_menu* handling_editor_menu::get() {
    static handling_editor_menu instance;
    return &instance;
}
