#pragma once
#include "rage/invoker/natives.h"
#include "game/camera_dir.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

// Shared "what am I pointing at" for the weapon menu.
//
// Ozark's gun features are all the same shape: when the player fires, work out
// what the shot would hit and do something to it. That is one raycast plus a
// shot-edge check, so it lives here once rather than in fifteen toggles.
namespace game::aim {

    struct hit {
        bool  valid;
        Entity entity;              // 0 when the shot hit scenery
        math::vector3<float> pos;
    };

    // True only on the frame the player starts shooting. IS_PED_SHOOTING stays
    // true for the whole burst, so acting on it directly fires a gun feature
    // dozens of times per trigger pull.
    inline bool just_fired() {
        static bool was_shooting = false;
        Ped ped = native::get_player_ped(-1);
        bool now = ped && native::is_ped_shooting(ped);
        bool edge = now && !was_shooting;
        was_shooting = now;
        return edge;
    }

    // Raycast from the camera along its facing. Ignores the player so aiming does
    // not simply hit yourself.
    inline hit trace(float distance = 500.f) {
        hit h = { false, 0, { 0.f, 0.f, 0.f } };

        Ped ped = native::get_player_ped(-1);
        if (!ped)
            return h;

        math::vector3<float> from = native::get_gameplay_cam_coord();
        math::vector3<float> dir  = game::camera_direction();

        math::vector3<float> to = {
            from.x + dir.x * distance,
            from.y + dir.y * distance,
            from.z + dir.z * distance
        };

        // The SYNCHRONOUS probe, deliberately. start_shape_test_los_probe is
        // asynchronous: it queues the query and the result is not available until
        // a later frame. This code used to call it and read the result in the same
        // breath, so get_shape_test_result always reported "still running", did_hit
        // stayed false, and every aim feature - laser sight, delete, force,
        // teleport, airstrike, instant kill - returned at the h.valid check having
        // done nothing. They were not broken individually; they all sat behind one
        // raycast that never landed.
        //
        // flags -1 = hit everything, and the player is excluded so aiming does not
        // simply resolve to yourself.
        int ray = native::start_expensive_synchronous_shape_test_los_probe(
                      from.x, from.y, from.z, to.x, to.y, to.z, -1, ped, 7);

        bool did_hit = false;
        Entity ent = 0;
        math::vector3<float> pos = { 0.f, 0.f, 0.f };
        math::vector3<float> normal = { 0.f, 0.f, 0.f };

        // And check the status this time. 2 is "complete"; anything else means the
        // out-params were not written and reading them is reading whatever was on
        // the stack. Ignoring this return is what let the original failure look
        // like "no hit" instead of "no answer".
        int status = native::get_shape_test_result(ray, &did_hit, &pos, &normal, &ent);
        if (status != 2)
            return h;

        h.valid = did_hit;
        h.entity = ent;
        h.pos = pos;
        return h;
    }
}
