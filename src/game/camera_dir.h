#pragma once
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "util/math.h"
#include "game/camera_dir_math.h"

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

    // Uses libm, deliberately, not native::sin / native::cos. Those are the
    // game's SIN and COS script commands and they take DEGREES. The code this
    // replaced converted to radians first and then handed the result to them,
    // so a 90 degree yaw arrived as 1.57 "degrees" and every direction came out
    // as very nearly {0, 1, 0} - always forward, always drifting slightly left.
    //
    // It survived unnoticed because get_gameplay_cam_rot was a {0,0,0} stub, and
    // sin(0)/cos(0) are 0 and 1 in either unit. Fixing the stub is what made the
    // unit bug visible. libm also spares two script-native calls per frame on a
    // path that runs while flying.
    inline math::vector3<float> camera_direction() {
        math::vector3<float> rot = native::get_gameplay_cam_rot(2);

        float x, y, z;
        camera_math::direction_from_rotation(rot.x, rot.z, &x, &y, &z);
        return { x, y, z };
    }
}
