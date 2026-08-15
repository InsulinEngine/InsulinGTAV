#pragma once
#include <math.h>

// Camera rotation to a direction vector. Kept free of the mini-STL and of every
// PS4 header so the host test can include it - the same split rainbow_math.h and
// color_math.h use.
//
// This exists as its own file because the bug it now guards against was live for
// months and invisible: the expansion used to call native::sin / native::cos,
// which are the game's SIN and COS script commands and take DEGREES, after
// converting its inputs to radians. A 90 degree yaw arrived as 1.57 "degrees",
// so every direction came out as very nearly {0, 1, 0} - forward, drifting
// slightly left, whatever the camera was doing. It hid behind a
// get_gameplay_cam_rot stub that returned {0,0,0}, where sin(0) and cos(0) agree
// in both units.
//
// Convention: the game is z-up, and yaw grows counter-clockwise from +Y. So +Y
// is forward, +X is right, and x therefore takes the NEGATIVE sine. Both callers
// (aim_ray and Superman flight) depend on that; change it here or nowhere.
namespace game::camera_math {

    inline void direction_from_rotation(float pitch_deg, float yaw_deg,
                                        float* out_x, float* out_y, float* out_z) {
        const float deg = 0.0174532924f;
        float pitch = pitch_deg * deg;
        float yaw   = yaw_deg   * deg;
        float cp    = cosf(pitch);

        *out_x = -sinf(yaw) * cp;
        *out_y =  cosf(yaw) * cp;
        *out_z =  sinf(pitch);
    }
}
