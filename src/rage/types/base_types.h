#pragma once
#include <stdint.h>

// Script handle types used by the generated natives.h. In the RAGE script ABI
// every handle is a 32-bit int; the names exist only for readability. Kept
// self-contained (no engine dependencies) on purpose so natives.h compiles
// against nothing but this file and invoker.h.

typedef int Void;      // void-returning natives return a dummy int slot
typedef int Any;
typedef int Ped;
typedef int Player;
typedef int Entity;
typedef int Vehicle;
typedef int Object;
typedef int ScrHandle;
typedef int Blip;
typedef int Cam;
typedef int Pickup;

typedef uint32_t Hash;

// math:: is the single authority for the vector types shared between the
// generated natives.h (which takes math::vector3<float>* out-params) and the
// ported Ozark menu code (which does vector2 arithmetic for 2D layout).
// vector3 is packed {x,y,z}: the 2D menu base never reads a native scrVector3
// result, so the PC scrVector3 padding is unnecessary here. util/math.h adds
// the remaining Ozark helpers (vector3_, matrix, within, ease, repeat) on top.
namespace math {
    template<typename T>
    struct vector2 {
        T x, y;

        // constexpr so namespace-scope vector2 globals (ui_vars positions/scales)
        // are constant-initialised. Without it they are dynamic-init, which
        // GoldHEN skips (no .init_array), zeroing every position/scale and
        // collapsing the whole menu to (0,0) size 0 -> invisible.
        constexpr vector2() : x(0), y(0) {}
        constexpr vector2(T in_x, T in_y) : x(in_x), y(in_y) {}

        vector2 operator*(float v) const { return vector2(x * v, y * v); }
        vector2 operator/(float v) const { return (v == 0.f) ? vector2(0, 0) : vector2(x / v, y / v); }
        vector2 operator+(float v) const { return vector2(x + v, y + v); }
        vector2 operator-(float v) const { return vector2(x - v, y - v); }
        vector2 operator*(const vector2& o) const { return vector2(x * o.x, y * o.y); }
        vector2 operator/(const vector2& o) const { return vector2(x / o.x, y / o.y); }
        vector2 operator+(const vector2& o) const { return vector2(x + o.x, y + o.y); }
        vector2 operator-(const vector2& o) const { return vector2(x - o.x, y - o.y); }
        vector2& operator+=(const vector2& o) { x += o.x; y += o.y; return *this; }
        vector2& operator-=(const vector2& o) { x -= o.x; y -= o.y; return *this; }
        vector2& operator+=(float v) { x += v; y += v; return *this; }
        vector2& operator-=(float v) { x -= v; y -= v; return *this; }
        vector2& operator*=(float v) { x *= v; y *= v; return *this; }
        bool operator==(const vector2& o) const { return x == o.x && y == o.y; }
    };

    template<typename T>
    struct vector3 {
        T x, y, z;

        constexpr vector3() : x(0), y(0), z(0) {}
        constexpr vector3(T in_x, T in_y, T in_z) : x(in_x), y(in_y), z(in_z) {}

        vector3 operator*(float v) const { return vector3(x * v, y * v, z * v); }
        vector3 operator+(const vector3& o) const { return vector3(x + o.x, y + o.y, z + o.z); }
        vector3 operator-(const vector3& o) const { return vector3(x - o.x, y - o.y, z - o.z); }
        vector3& operator+=(const vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
        bool operator==(const vector3& o) const { return x == o.x && y == o.y && z == o.z; }
    };

    template<typename T>
    struct vector4 { T x, y, z, w; };

    template<typename T> inline T clamp(T v, T lo, T hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
}
