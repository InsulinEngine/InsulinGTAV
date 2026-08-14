#include "menu/base/submenus/vehicle_plate.h"
#include "menu/base/submenus/vehicle_customs.h"
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
    int g_style = 0;
}

void vehicle_plate_menu::load() {
    set_name("Number Plate");
    set_parent<vehicle_customs_menu>();

    add_option(number_option<int>(SCROLLSELECT, "Plate Style")
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
}

vehicle_plate_menu* vehicle_plate_menu::get() {
    static vehicle_plate_menu instance;
    return &instance;
}
