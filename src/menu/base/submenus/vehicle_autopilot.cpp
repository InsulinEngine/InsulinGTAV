#include "menu/base/submenus/vehicle_autopilot.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    // Everything here acts on the vehicle you are sitting in. Nothing applies on
    // foot, so each entry resolves it first rather than assuming one is there.
    Vehicle my_vehicle() {
        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return 0;
        return native::get_vehicle_ped_is_in(ped, false);
    }
}

void vehicle_autopilot_menu::load() {
    set_name("Autopilot");
    set_parent<vehicle_menu>();

    add_option(button_option("Enable Autopilot")
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
}

void vehicle_autopilot_menu::feature_update() {
}

vehicle_autopilot_menu* vehicle_autopilot_menu::get() {
    static vehicle_autopilot_menu instance;
    return &instance;
}
