#include "menu/panels/vehicle_panel.h"
#include "util/num_to_string.h"
#include "rage/invoker/natives.h"

namespace menu::panels {
    math::vector2<float> vehicle_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return p.get_render_scale();

        Vehicle veh = native::get_vehicle_ped_is_in(ped, false);
        if (!veh) return p.get_render_scale();

        char buf[64];

        // The raw text label, e.g. ADDER: GET_LABEL_TEXT is in no header on this
        // build, so there is nothing to resolve it to a friendly name with.
        p.item_full("Model", native::get_display_name_from_vehicle_model(native::get_entity_model(veh)));
        p.item("Speed",  util::ftos(native::get_entity_speed(veh) * 3.6f, 1, buf, sizeof(buf)));
        p.item("Engine", util::ftos(native::get_vehicle_engine_health(veh), 0, buf, sizeof(buf)));
        p.item("Body",   util::ftos(native::get_vehicle_body_health(veh), 0, buf, sizeof(buf)));
        p.item("Handle", util::itos((int)veh, buf, sizeof(buf)));

        return p.get_render_scale();
    }
}
