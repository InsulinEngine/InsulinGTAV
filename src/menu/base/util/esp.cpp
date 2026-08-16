#include "menu/base/util/esp.h"
#include "menu/base/util/esp_math.h"
#include "menu/base/renderer.h"
#include "game/player_list.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/hash_natives.h"
#include "platform/log.h"

#include <stdio.h>
#include <math.h>

namespace menu::esp {

    namespace {
        // Per-frame budget. Reset by begin_frame(); consumers draw nearest
        // first, so what gets dropped when the cap bites is the least useful.
        constexpr int  k_frame_cap        = 48;
        int            g_drawn            = 0;
        bool           g_cap_reported     = false;

        // Vertical offsets for the two projections the 2D box is built from,
        // in metres relative to the entity origin. A ped's origin sits at its
        // feet-ish; these are 2take1's values and read correctly for humans.
        constexpr float k_feet_offset = -1.0f;
        constexpr float k_head_offset =  0.8f;

        bool project(const math::vector3<float>& world, math::vector2<float>* out) {
            return native::get_screen_coord_from_world_coord(world.x, world.y, world.z,
                                                            &out->x, &out->y);
        }

        float distance_to_local(const math::vector3<float>& coords) {
            game::players::entry me = game::players::get(game::players::local_id());
            if (!me.ped) return 0.f;
            math::vector3<float> mine = native::get_entity_coords(me.ped, false);
            float dx = coords.x - mine.x, dy = coords.y - mine.y, dz = coords.z - mine.z;
            return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        }
    }

    void begin_frame() {
        g_drawn = 0;
    }

    int drawn_this_frame() { return g_drawn; }
    int frame_cap()        { return k_frame_cap; }

    static void name_esp(const esp_context& ctx, Entity entity,
                         const math::vector3<float>& coords, float distance,
                         const char* name_override) {
        math::vector2<float> head;
        math::vector3<float> head_world = { coords.x, coords.y, coords.z + k_head_offset };
        if (!project(head_world, &head)) return;

        char text[64];
        if (name_override && name_override[0])
            snprintf(text, sizeof(text), "%s", name_override);
        else
            snprintf(text, sizeof(text), "%08X", native::get_entity_model(entity));

        const float scale = esp_math::distance_scale(distance, ctx.m_max_distance, 0.25f, 0.15f);
        const float height = scale / 10.f;
        const float pad = 0.002f;

        float width = menu::renderer::calculate_string_width(text, 0, scale) + (pad * 2.f);
        menu::renderer::draw_rect({ head.x - (width * 0.5f), head.y - height },
                                  { width, height }, ctx.m_name_bg_color);
        menu::renderer::draw_text(text, { head.x, head.y - height + pad }, scale, 0,
                                  ctx.m_name_text_color, JUSTIFY_CENTER);

        if (ctx.m_name_type == 1) {
            char dist_text[32];
            snprintf(dist_text, sizeof(dist_text), "%dm", (int)distance);
            float dw = menu::renderer::calculate_string_width(dist_text, 0, scale) + (pad * 2.f);
            float y = head.y - (height * 2.f);
            menu::renderer::draw_rect({ head.x - (dw * 0.5f), y }, { dw, height }, ctx.m_name_bg_color);
            menu::renderer::draw_text(dist_text, { head.x, y + pad }, scale, 0,
                                      ctx.m_name_text_color, JUSTIFY_CENTER);
        }
    }

    static void snapline_esp(const esp_context& ctx, const math::vector3<float>& coords) {
        game::players::entry me = game::players::get(game::players::local_id());
        if (!me.ped) return;
        math::vector3<float> mine = native::get_entity_coords(me.ped, false);
        menu::renderer::draw_line(mine, coords, ctx.m_snapline_color);
    }

    void draw_entity(const esp_context& ctx, Entity entity, const char* name_override) {
        if (!ctx.any() || !entity) return;
        if (!native::does_entity_exist(entity)) return;

        math::vector3<float> coords = native::get_entity_coords(entity, false);
        if (coords.x == 0.f && coords.y == 0.f && coords.z == 0.f) return;

        const float distance = distance_to_local(coords);
        if (distance > (float)ctx.m_max_distance) return;

        if (g_drawn >= k_frame_cap) {
            // Say so once per session rather than per frame: a silent cap looks
            // exactly like an ESP that does not work.
            if (!g_cap_reported) {
                g_cap_reported = true;
                LOG_WARN("esp: frame cap of %d entities reached; further entities "
                         "this frame are skipped", k_frame_cap);
            }
            return;
        }
        g_drawn++;

        if (ctx.m_snapline) snapline_esp(ctx, coords);
        if (ctx.m_name)     name_esp(ctx, entity, coords, distance, name_override);
    }
}
