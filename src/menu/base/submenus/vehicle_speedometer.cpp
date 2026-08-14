#include "menu/base/submenus/vehicle_speedometer.h"
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

    bool g_plate = false;
    int g_last_kmh = -1;
}

void vehicle_speedometer_menu::load() {
    set_name("Speedometer");
    set_parent<vehicle_menu>();

    add_option(toggle_option("Numberplate Speedometer")
        .add_toggle(g_plate)
        .add_tooltip("Writes your speed onto the number plate")
        .add_savable(get_submenu_name_stack()));
}

void vehicle_speedometer_menu::feature_update() {
    Vehicle v = my_vehicle();
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
}

vehicle_speedometer_menu* vehicle_speedometer_menu::get() {
    static vehicle_speedometer_menu instance;
    return &instance;
}
