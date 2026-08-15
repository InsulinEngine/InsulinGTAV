#include "menu/panels/player_panel.h"
#include "util/num_to_string.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

namespace menu::panels {
    // m_update is a plain function pointer, so there is no capture to worry
    // about here - unlike every option handler in this codebase.
    math::vector2<float> player_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        Ped ped = native::get_player_ped(-1);
        if (!ped) return p.get_render_scale();   // framework draws nothing

        char buf[64];
        math::vector3<float> c = native::get_entity_coords(ped, true);

        p.item("X",       util::ftos(c.x, 2, buf, sizeof(buf)));
        p.item("Y",       util::ftos(c.y, 2, buf, sizeof(buf)));
        p.item("Z",       util::ftos(c.z, 2, buf, sizeof(buf)));
        p.item("Heading", util::ftos(native::get_entity_heading(ped), 1, buf, sizeof(buf)));
        p.item("Health",  util::itos(native::get_entity_health(ped), buf, sizeof(buf)));
        p.item("Armour",  util::itos(native::get_ped_armour(ped), buf, sizeof(buf)));
        p.item("Wanted",  util::itos(native::get_player_wanted_level(native::player_id()), buf, sizeof(buf)));
        p.item_full("Zone", native::get_name_of_zone(c.x, c.y, c.z));

        return p.get_render_scale();
    }
}
