#include "menu/base/submenus/vehicle_modifiers.h"
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

void vehicle_modifiers_menu::load() {
    set_name("Modifiers");
    set_parent<vehicle_menu>();

    add_option(button_option("Boost Forward")
        .add_click([] {
            Vehicle v = my_vehicle();
            if (!v) return;
            math::vector3<float> f = native::get_entity_forward_vector(v);
            native::apply_force_to_entity(v, 1, f.x * 60.f, f.y * 60.f, 0.f,
                                          0.f, 0.f, 0.f, 0, true, true, true, false, true);
        }));
}

void vehicle_modifiers_menu::feature_update() {
}

vehicle_modifiers_menu* vehicle_modifiers_menu::get() {
    static vehicle_modifiers_menu instance;
    return &instance;
}
