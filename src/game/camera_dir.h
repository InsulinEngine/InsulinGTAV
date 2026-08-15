#pragma once
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "util/math.h"

// Where the gameplay camera is looking, as a unit vector.
//
// Shared because two unrelated features need the same answer: aim_ray (what
// would this shot hit) and Superman flight (which way is forward). They were
// about to hold one copy each, and a copy of a coordinate convention is the kind
// of duplicate that silently disagrees after the first correction to either.
//
// Convention: the game is z-up and its yaw grows counter-clockwise from +Y, so
// the expansion below is not the textbook one - +Y is forward, and x takes the
// negative sine. Both callers depend on that, so change it in one place or not
// at all.
namespace game {

    inline math::vector3<float> camera_direction() {
        math::vector3<float> rot = native::get_gameplay_cam_rot(2);

        const float deg = 0.0174532924f;
        float pitch = rot.x * deg;
        float yaw   = rot.z * deg;
        float cp    = native::cos(pitch);

        return {
            -native::sin(yaw) * cp,
             native::cos(yaw) * cp,
             native::sin(pitch)
        };
    }
}
