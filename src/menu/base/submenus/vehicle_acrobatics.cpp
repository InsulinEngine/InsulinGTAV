#include "menu/base/submenus/vehicle_acrobatics.h"
#include "menu/base/submenus/vehicle_movement.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }
    float g_force = 20.f;

    // Rotation impulses are applied about the vehicle's own axes, which is what
    // makes them read as a roll or a flip rather than a shove in world space.
    void spin(float rx, float ry, float rz) {
        Vehicle v = my_vehicle();
        if (!v) return;
        native::apply_force_to_entity(v, 1, 0.f, 0.f, 0.f,
                                      rx * g_force, ry * g_force, rz * g_force,
                                      0, true, true, true, false, true);
    }
}

void vehicle_acrobatics_menu::load() {
    set_name("Acrobatics");
    set_parent<vehicle_movement_menu>();

    add_option(number_option<float>(SCROLLSELECT, "Force")
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
}

vehicle_acrobatics_menu* vehicle_acrobatics_menu::get() {
    static vehicle_acrobatics_menu instance;
    return &instance;
}
