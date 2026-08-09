#pragma once
#include "platform/stdafx.h"   // brings math::vector2/vector3/vector4, clamp, lerp
#include <math.h>

// Additive Ozark math helpers on top of base_types' math:: (which is the single
// authority for vector2/vector3/vector4/clamp/lerp). Only the pieces the Ozark
// base actually needs beyond those, plus a couple of harmless extras kept for
// parity with later feature ports.
#define PI (float)3.14159265358979323846264338327950288

namespace math {
    // Packed serialized vector3 (no scrVector padding). color_rgba::as_vector
    // returns vector3_<int>; also the target of vector3::to_serialized on PC.
    template<typename T>
    struct vector3_ {
        T x, y, z;

        vector3_() : x(0), y(0), z(0) {}
        vector3_(T in_x, T in_y, T in_z) : x(in_x), y(in_y), z(in_z) {}

        vector3_ operator*(float v) const { return vector3_(x * v, y * v, z * v); }
        vector3_ operator+(const vector3_& o) const { return vector3_(x + o.x, y + o.y, z + o.z); }
        vector3_ operator-(const vector3_& o) const { return vector3_(x - o.x, y - o.y, z - o.z); }
        bool operator==(const vector3_& o) const { return x == o.x && y == o.y && z == o.z; }

        static vector3<T> to_padded(vector3_<T> v) { return vector3<T>(v.x, v.y, v.z); }
        float get_length() { return (float)sqrt((x * x) + (y * y) + (z * z)); }
    };

    template<typename T>
    struct matrix {
        union {
            struct { vector4<T> m_left, m_up, m_forward, m_translation; };
            T m_elements[4][4];
        };
        matrix() {}
        T& operator()(int row, int col) { return m_elements[row][col]; }
    };

    template<typename T>
    bool within(T val, T min, T max) { return val <= max && val >= min; }

    inline void ease(float& toEase, float& easeFrom, float multiplier) {
        toEase += toEase < easeFrom ? fabsf(toEase - easeFrom) / multiplier
                                    : -fabsf(toEase - easeFrom) / multiplier;
    }

    inline float repeat(float t, float length) {
        return clamp(t - (float)floor(t / length) * length, 0.f, length);
    }
}
