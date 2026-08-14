#include "menu/panels/world_panel.h"
#include "util/num_to_string.h"
#include "rage/invoker/natives.h"

namespace menu::panels {
    math::vector2<float> world_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        char buf[64];
        char clock[16];
        snprintf(clock, sizeof(clock), "%02d:%02d",
                 native::get_clock_hours(), native::get_clock_minutes());

        p.item("Time",    clock);
        p.item("Weather", util::itos((int)native::get_prev_weather_type_hash_name(), buf, sizeof(buf)));

        return p.get_render_scale();
    }
}
