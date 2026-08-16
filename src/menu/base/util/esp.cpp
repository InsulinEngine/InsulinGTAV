#include "menu/base/util/esp.h"
#include "menu/base/util/esp_math.h"
#include "menu/base/renderer.h"
#include "game/player_list.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/hash_natives.h"
#include "rage/ped_bones.h"
#include "platform/log.h"

#include <stdio.h>
#include <math.h>

namespace menu::esp {

    namespace {
        // Per-frame budget. Reset by begin_frame(); consumers draw nearest
        // first, so what gets dropped when the cap bites is the least useful.
        constexpr int  k_frame_cap        = 48;
        int            g_drawn            = 0;

        // The ESP's own frame clock, advanced by begin_frame() and by nothing
        // else. rage::gfx::watch_frame() would have done, except that it stops
        // dead at 0 until a texture dictionary has been committed (gfx.cpp
        // returns early while g_watch_slot < 0), so a boot with no menu image
        // would leave every interval below permanently unexpired.
        uint32_t       g_frame            = 0;

        // Cap reporting, throttled rather than one-shot: a silent cap looks
        // exactly like an ESP that does not work, and a cap that bites for the
        // first time an hour into a session deserves to be said out loud again.
        constexpr uint32_t k_cap_report_interval = 1800;   // ~1 minute at 30 fps
        bool           g_cap_reported     = false;
        uint32_t       g_cap_report_frame = 0;

        // The local player, resolved once per frame by resolve_local_player()
        // rather than once per candidate entity. g_local_frame is the frame the
        // cache was filled on: draw_entity refuses to use it on any other frame,
        // so a frame where the resolve did not run (overlay up, no local player
        // yet, a consumer called from outside the tick) draws nothing rather
        // than drawing against last frame's origin.
        Ped                  g_local_ped    = 0;
        math::vector3<float> g_local_coords;
        uint32_t             g_local_frame  = 0;

        // Vertical offsets for the two projections the 2D box is built from,
        // in metres relative to the entity origin. A ped's origin sits at its
        // feet-ish; these are 2take1's values and read correctly for humans.
        constexpr float k_feet_offset = -1.0f;
        constexpr float k_head_offset =  0.8f;

        bool project(const math::vector3<float>& world, math::vector2<float>* out) {
            return native::get_screen_coord_from_world_coord(world.x, world.y, world.z,
                                                            &out->x, &out->y);
        }

        float distance_between(const math::vector3<float>& a, const math::vector3<float>& b) {
            float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
            return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        }

        // head is already projected by the caller (draw_entity gates the frame
        // budget on that same projection), so this never touches the screen
        // conversion natives itself.
        void name_esp(const esp_context& ctx, Entity entity,
                     const math::vector2<float>& head, float distance,
                     const char* name_override) {
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

        // local_coords is resolved once by draw_entity and handed down, rather
        // than re-fetched here.
        void snapline_esp(const esp_context& ctx, const math::vector3<float>& local_coords,
                          const math::vector3<float>& coords) {
            menu::renderer::draw_line(local_coords, coords, ctx.m_snapline_color);
        }

        // The box the whole feature is named for. Ozark left this unimplemented and
        // its menu entries commented out; the recipe is 2take1's.
        // head is already projected by draw_entity (it gates the frame budget on
        // that same projection) and handed down; only the feet still need it.
        bool entity_box(const math::vector2<float>& head, const math::vector3<float>& coords,
                        esp_math::box2d* out) {
            math::vector2<float> feet;
            math::vector3<float> feet_world = { coords.x, coords.y, coords.z + k_feet_offset };
            if (!project(feet_world, &feet)) return false;
            *out = esp_math::box_from_projections(head.x, head.y, feet.y);
            return true;
        }

        void box_2d_esp(const esp_context& ctx, const esp_math::box2d& b) {
            // draw_outlined_rect already draws a fill plus a border and gets the
            // corner maths right; a fully transparent fill turns it into a frame.
            const float thickness = b.h * 0.006f;
            menu::renderer::draw_outlined_rect({ b.x, b.y }, { b.w, b.h }, thickness,
                                               color_rgba(0, 0, 0, 0), ctx.m_2d_box_color);
        }

        void corners_2d_esp(const esp_context& ctx, const esp_math::box2d& b) {
            const float t   = b.h * 0.006f;          // line thickness
            const float len_x = b.w * 0.30f;         // how far a corner reaches
            const float len_y = b.h * 0.20f;
            const color_rgba c = ctx.m_2d_corners_color;

            const float l = b.x, r = b.x + b.w - t, top = b.y, bot = b.y + b.h - t;

            // Each corner is one horizontal and one vertical stub.
            menu::renderer::draw_rect({ l, top },             { len_x, t }, c);
            menu::renderer::draw_rect({ l, top },             { t, len_y }, c);
            menu::renderer::draw_rect({ r - len_x + t, top }, { len_x, t }, c);
            menu::renderer::draw_rect({ r, top },             { t, len_y }, c);
            menu::renderer::draw_rect({ l, bot },             { len_x, t }, c);
            menu::renderer::draw_rect({ l, bot - len_y + t }, { t, len_y }, c);
            menu::renderer::draw_rect({ r - len_x + t, bot }, { len_x, t }, c);
            menu::renderer::draw_rect({ r, bot - len_y + t }, { t, len_y }, c);
        }

        void healthbar_esp(const esp_context& ctx, Entity entity,
                           const esp_math::box2d& b) {
            const int  health     = native::get_entity_health(entity);
            const int  max        = native::get_entity_max_health(entity);
            const bool is_ped     = native::is_entity_a_ped(entity);
            const int  armour     = is_ped ? native::get_ped_armour(entity) : 0;
            const int  armour_max = is_ped ? 50 : 0;
            const float f = esp_math::health_fraction(health, max, armour, armour_max);

            const float w   = b.w * 0.12f;
            const float gap = b.w * 0.10f;
            const float x   = b.x - gap - w;

            // Background the full height, then the filled part growing from the
            // bottom, so a draining bar shortens downward as players expect.
            menu::renderer::draw_rect({ x, b.y }, { w, b.h }, color_rgba(0, 0, 0, 180));
            const float fh = b.h * f;
            menu::renderer::draw_rect({ x, b.y + (b.h - fh) }, { w, fh }, ctx.m_healthbar_color);
        }

        // The model's own half-extents, fetched once for whichever of the two
        // world-space elements are on: both used to call get_entity_model plus
        // get_model_dimensions for themselves, which is two natives paid twice
        // for one answer when both are enabled. Returns false for degenerate
        // bounds - a model with no usable dimensions draws neither element.
        bool model_radii(Entity entity, float* rx, float* ry, float* rz) {
            math::vector3<float> lo, hi;
            native::get_model_dimensions(native::get_entity_model(entity), &lo, &hi);

            *rx = (hi.x - lo.x) * 0.5f;
            *ry = (hi.y - lo.y) * 0.5f;
            *rz = (hi.z - lo.z) * 0.5f;
            return *rx > 0.f && *ry > 0.f && *rz > 0.f;
        }

        // Ozark's 3D box: the model's own bounds turned into eight world-space
        // corners, joined by twelve edges and the spokes Ozark draws from the
        // centre, which are what make it read as a solid at a distance.
        void box_3d_esp(const esp_context& ctx, Entity entity,
                        const math::vector3<float>& coords,
                        float rx, float ry, float rz) {
            const color_rgba c = ctx.m_3d_box_color;
            math::vector3<float> FUL = native::get_offset_from_entity_in_world_coords(entity, -rx,  ry,  rz);
            math::vector3<float> FUR = native::get_offset_from_entity_in_world_coords(entity,  rx,  ry,  rz);
            math::vector3<float> FBL = native::get_offset_from_entity_in_world_coords(entity, -rx,  ry, -rz);
            math::vector3<float> FBR = native::get_offset_from_entity_in_world_coords(entity,  rx,  ry, -rz);
            math::vector3<float> BUL = native::get_offset_from_entity_in_world_coords(entity, -rx, -ry,  rz);
            math::vector3<float> BUR = native::get_offset_from_entity_in_world_coords(entity,  rx, -ry,  rz);
            math::vector3<float> BBL = native::get_offset_from_entity_in_world_coords(entity, -rx, -ry, -rz);
            math::vector3<float> BBR = native::get_offset_from_entity_in_world_coords(entity,  rx, -ry, -rz);

            menu::renderer::draw_line(FBL, FUL, c);
            menu::renderer::draw_line(FBR, FUR, c);
            menu::renderer::draw_line(BBL, BUL, c);
            menu::renderer::draw_line(BBR, BUR, c);
            menu::renderer::draw_line(FUL, FUR, c);
            menu::renderer::draw_line(FBL, FBR, c);
            menu::renderer::draw_line(BUL, BUR, c);
            menu::renderer::draw_line(BBL, BBR, c);
            menu::renderer::draw_line(FUL, BUL, c);
            menu::renderer::draw_line(FUR, BUR, c);
            menu::renderer::draw_line(FBL, BBL, c);
            menu::renderer::draw_line(FBR, BBR, c);

            menu::renderer::draw_line(coords, FUL, c);
            menu::renderer::draw_line(coords, FUR, c);
            menu::renderer::draw_line(coords, FBL, c);
            menu::renderer::draw_line(coords, FBR, c);
            menu::renderer::draw_line(coords, BUL, c);
            menu::renderer::draw_line(coords, BUR, c);
            menu::renderer::draw_line(coords, BBL, c);
            menu::renderer::draw_line(coords, BBR, c);
        }

        // Three long axes through the entity, coloured X red, Y green, Z blue.
        // Ozark's fixed colours: they identify an axis, so they are not themeable.
        // Takes the half-extents model_radii() already validated, so - like the
        // 3D box - a model with degenerate bounds draws nothing instead of three
        // zero-length axes stacked on the entity's origin.
        void axis_3d_esp(Entity entity, float rx, float ry) {
            const float dx = rx * 4.f;   // == (hi.x - lo.x) * 2, exactly
            const float dy = ry * 4.f;

            math::vector3<float> XL = native::get_offset_from_entity_in_world_coords(entity, -dx, 0.f, 0.f);
            math::vector3<float> XR = native::get_offset_from_entity_in_world_coords(entity,  dx, 0.f, 0.f);
            math::vector3<float> YF = native::get_offset_from_entity_in_world_coords(entity, 0.f,  dy, 0.f);
            math::vector3<float> YB = native::get_offset_from_entity_in_world_coords(entity, 0.f, -dy, 0.f);
            math::vector3<float> ZU = native::get_offset_from_entity_in_world_coords(entity, 0.f, 0.f,  500.f);
            math::vector3<float> ZD = native::get_offset_from_entity_in_world_coords(entity, 0.f, 0.f, -500.f);

            menu::renderer::draw_line(XL, XR, color_rgba(255, 0, 0, 255));
            menu::renderer::draw_line(YF, YB, color_rgba(0, 255, 0, 255));
            menu::renderer::draw_line(ZU, ZD, color_rgba(0, 0, 255, 255));
        }

        // The fifteen unique joints the skeleton is built from. Everything below
        // indexes this table rather than naming bone ids, so each joint is
        // fetched exactly once per ped per frame however many bones share it and
        // whichever of the two elements are on.
        enum joint {
            J_HEAD, J_NECK, J_PELVIS,
            J_L_UPPERARM, J_R_UPPERARM,
            J_L_FOREARM,  J_R_FOREARM,
            J_L_HAND,     J_R_HAND,
            J_L_KNEE,     J_R_KNEE,
            J_L_FOOT,     J_R_FOOT,
            J_L_TOE,      J_R_TOE,
            J_COUNT
        };

        void skeleton_esp(const esp_context& ctx, Entity ped, bool bones, bool joints) {
            using namespace rage::ped_bones;

            // Same fifteen ids as before, in enum order.
            static const int k_joints[J_COUNT] = {
                SKEL_Head, SKEL_Neck_1, SKEL_Pelvis,
                SKEL_L_UpperArm, SKEL_R_UpperArm,
                SKEL_L_Forearm,  SKEL_R_Forearm,
                SKEL_L_Hand,     SKEL_R_Hand,
                MH_L_Knee,       MH_R_Knee,
                SKEL_L_Foot,     SKEL_R_Foot,
                SKEL_L_Toe0,     SKEL_R_Toe0,
            };

            // Ozark's fourteen bones, as index pairs into k_joints. Ids would
            // read more directly, but they are what made this fetch a shared
            // joint once per pair it appears in: SKEL_Neck_1 is an endpoint of
            // four bones, and was fetched four times.
            static const unsigned char k_bones[][2] = {
                { J_R_FOOT, J_R_KNEE }, { J_R_TOE, J_R_FOOT },
                { J_L_TOE, J_L_FOOT },  { J_L_FOOT, J_L_KNEE },
                { J_R_KNEE, J_PELVIS }, { J_L_KNEE, J_PELVIS },
                { J_PELVIS, J_NECK },
                { J_NECK, J_R_UPPERARM }, { J_NECK, J_L_UPPERARM },
                { J_R_UPPERARM, J_R_FOREARM }, { J_L_UPPERARM, J_L_FOREARM },
                { J_R_FOREARM, J_R_HAND }, { J_L_FOREARM, J_L_HAND },
                { J_NECK, J_HEAD },
            };
            const int bone_count = (int)(sizeof(k_bones) / sizeof(k_bones[0]));

            // Fifteen bone-coord natives and fifteen projections, once, up
            // front. get_ped_bone_coords is a hash native and the most expensive
            // call in this file; the projection of a world point is a pure
            // function of that point, so sharing it changes no geometry.
            math::vector3<float> world[J_COUNT];
            math::vector2<float> screen[J_COUNT];
            bool                 on_screen[J_COUNT];
            for (int i = 0; i < J_COUNT; i++) {
                world[i] = native::get_ped_bone_coords(ped, k_joints[i], 0.f, 0.f, 0.f);
                on_screen[i] = project(world[i], &screen[i]);
            }

            // Bones first, then joints - the order draw_entity used when it
            // called this twice.
            if (bones) {
                const color_rgba c = ctx.m_skeleton_bones_color;
                for (int i = 0; i < bone_count; i++) {
                    const int a = k_bones[i][0], b = k_bones[i][1];
                    if (!on_screen[a] || !on_screen[b]) continue;
                    menu::renderer::draw_line_2d({ screen[a].x, screen[a].y, 0.f },
                                                 { screen[b].x, screen[b].y, 0.f }, c);
                }
            }

            if (joints) {
                const color_rgba c = ctx.m_skeleton_joints_color;
                for (int i = 0; i < J_COUNT; i++) {
                    if (!on_screen[i]) continue;   // off-screen joints cost nothing
                    native::draw_marker(28, world[i].x, world[i].y, world[i].z,
                                        0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                        0.03f, 0.03f, 0.03f, c.r, c.g, c.b, c.a,
                                        0, 0, 0, 0, nullptr, nullptr, 0);
                }
            }
        }

        void weapon_esp(const esp_context& ctx, Entity ped) {
            math::vector3<float> hand =
                native::get_ped_bone_coords(ped, rage::ped_bones::SKEL_R_Hand, 0.f, 0.f, 0.f);
            math::vector2<float> s;
            if (!project(hand, &s)) return;
            native::draw_marker(28, hand.x, hand.y, hand.z, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                0.05f, 0.05f, 0.05f,
                                ctx.m_weapon_color.r, ctx.m_weapon_color.g,
                                ctx.m_weapon_color.b, ctx.m_weapon_color.a,
                                0, 0, 0, 0, nullptr, nullptr, 0);
        }
    }

    void begin_frame() {
        g_frame++;
        g_drawn = 0;
    }

    void resolve_local_player() {
        // Five natives plus a coord fetch, once per frame. This used to run per
        // candidate entity inside draw_entity - including for every entity the
        // distance cull discarded a line later, which on a 200-vehicle pool was
        // ~1,200 native calls a frame that drew nothing.
        game::players::entry me = game::players::get(game::players::local_id());
        if (!me.ped) {
            g_local_ped = 0;        // stamp left behind: draw_entity draws nothing
            return;
        }
        g_local_coords = native::get_entity_coords(me.ped, false);
        g_local_ped    = me.ped;
        g_local_frame  = g_frame;
    }

    int drawn_this_frame() { return g_drawn; }
    int frame_cap()        { return k_frame_cap; }

    void draw_entity(const esp_context& ctx, Entity entity, const char* name_override) {
        if (!ctx.any() || !entity) return;

        // Every element below is defined relative to the local player - there
        // is nothing meaningful to draw without one. Cached by
        // resolve_local_player() at the top of this frame's feature pass; a
        // stamp from any earlier frame means that pass did not run (the overlay
        // is up, the local player is not valid yet, or this call came from
        // outside the tick), and last frame's origin is not something to draw
        // a snapline or a distance cull against.
        if (!g_local_ped || g_local_frame != g_frame) return;
        const math::vector3<float>& local_coords = g_local_coords;

        if (!native::does_entity_exist(entity)) return;

        math::vector3<float> coords = native::get_entity_coords(entity, false);
        if (coords.x == 0.f && coords.y == 0.f && coords.z == 0.f) return;

        const float distance = distance_between(coords, local_coords);
        if (distance > (float)ctx.m_max_distance) return;

        // Gate the frame budget on the entity actually landing on screen: an
        // entity that draws nothing costs no frame time and should not take a
        // slot ahead of one that is visible. Computed once and reused by
        // name_esp below rather than projected a second time.
        math::vector2<float> head;
        math::vector3<float> head_world = { coords.x, coords.y, coords.z + k_head_offset };
        if (!project(head_world, &head)) return;

        if (g_drawn >= k_frame_cap) {
            // Throttled, not one-shot: a silent cap looks exactly like an ESP
            // that does not work, and the first time it bites is not necessarily
            // the only time worth hearing about - a lobby that fills an hour in
            // would otherwise say nothing at all.
            if (!g_cap_reported || (g_frame - g_cap_report_frame) >= k_cap_report_interval) {
                g_cap_reported     = true;
                g_cap_report_frame = g_frame;
                LOG_WARN("esp: frame cap of %d entities reached; further entities "
                         "this frame are skipped", k_frame_cap);
            }
            return;
        }
        g_drawn++;

        if (ctx.m_snapline) snapline_esp(ctx, local_coords, coords);

        if (ctx.m_2d_box || ctx.m_2d_corners || ctx.m_healthbar) {
            esp_math::box2d b;
            if (entity_box(head, coords, &b)) {
                if (ctx.m_2d_box)     box_2d_esp(ctx, b);
                if (ctx.m_2d_corners) corners_2d_esp(ctx, b);
                if (ctx.m_healthbar)  healthbar_esp(ctx, entity, b);
            }
        }

        // One model lookup for both world-space elements, and one degenerate
        // -bounds guard covering both: they are built from the same two natives
        // and the same half-extents.
        if (ctx.m_3d_box || ctx.m_3d_axis) {
            float rx, ry, rz;
            if (model_radii(entity, &rx, &ry, &rz)) {
                if (ctx.m_3d_box)  box_3d_esp(ctx, entity, coords, rx, ry, rz);
                if (ctx.m_3d_axis) axis_3d_esp(entity, rx, ry);
            }
        }

        // Bones are the most expensive thing here by an order of magnitude -
        // fifteen hash natives per ped per frame - so they carry their own, much
        // nearer radius, and they are skipped entirely until the hash table has
        // been recovered rather than calling into an empty table.
        const bool bones_possible = ctx.m_ped
                                 && rage::hash_natives::usable()
                                 && distance <= (float)ctx.m_skeleton_distance
                                 && native::is_entity_a_ped(entity);
        if (bones_possible) {
            // One call for both elements: the fifteen joints they share are
            // fetched once whether one or both are on.
            if (ctx.m_skeleton_bones || ctx.m_skeleton_joints)
                skeleton_esp(ctx, entity, ctx.m_skeleton_bones, ctx.m_skeleton_joints);
            if (ctx.m_weapon) weapon_esp(ctx, entity);
        }

        if (ctx.m_name) name_esp(ctx, entity, head, distance, name_override);
    }
}
