#include "menu/base/submenus/vehicle_seats.h"
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

    Ped me() { return native::get_player_ped(-1); }
}

void vehicle_seats_menu::load() {
    set_name("Seats");
    set_parent<vehicle_menu>();

    add_option(button_option("Kick All Seats")
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
}

void vehicle_seats_menu::feature_update() {
}

vehicle_seats_menu* vehicle_seats_menu::get() {
    static vehicle_seats_menu instance;
    return &instance;
}
