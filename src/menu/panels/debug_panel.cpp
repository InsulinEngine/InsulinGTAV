#include "menu/panels/debug_panel.h"
#include "util/num_to_string.h"
#include "platform/build_tag.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/invoker.h"
#include "rage/invoker/hash_natives.h"

namespace menu::panels {
    namespace {
        // Smoothed so the number is readable rather than a blur. m_custom_ptr is
        // the framework's per-panel storage, but this is a single float shared by
        // the one debug panel, so a file static is honest and simpler.
        float g_fps = 0.f;
    }

    math::vector2<float> debug_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        float dt = native::get_frame_time();
        if (dt > 0.f) {
            float instant = 1.f / dt;
            g_fps = (g_fps == 0.f) ? instant : (g_fps * 0.9f + instant * 0.1f);
        }

        char buf[64];
        char base[32];
        snprintf(base, sizeof(base), "0x%llx",
                 (unsigned long long)rage::invoker::g_eboot_base);

        p.item_full("Build", INSULIN_BUILD_TAG);
        p.item("Base",    base);
        p.item("FPS",     util::ftos(g_fps, 0, buf, sizeof(buf)));
        p.item("Natives", util::itos((int)rage::hash_natives::entry_count(), buf, sizeof(buf)));
        p.item("Table",   rage::hash_natives::usable() ? "usable" : "pending");

        return p.get_render_scale();
    }
}
