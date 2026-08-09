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

// A few natives take a coordinate out-pointer, which Ozark's headers spell as
// math::vector3<float>*. Only the layout matters here.
namespace math {
	template<typename T> struct vector3 { T x, y, z; };
	template<typename T> struct vector4 { T x, y, z, w; };
	template<typename T> struct vector2 { T x, y; };

	template<typename T> inline T clamp(T v, T lo, T hi) {
		return v < lo ? lo : (v > hi ? hi : v);
	}
	inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
}
