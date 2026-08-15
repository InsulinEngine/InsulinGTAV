#pragma once
#include <math.h>

// RGB/HSV conversion, kept free of ui_vars and the mini-STL so the host test can
// include it. menu::renderer wraps these back up in color_rgba/color_hsv.
namespace menu::color_math {

    struct hsv { float h, s, v; };

    inline hsv rgb_to_hsv(int ri, int gi, int bi) {
        float r = ri / 255.0f, g = gi / 255.0f, b = bi / 255.0f;
        float max = fmaxf(r, fmaxf(g, b));
        float min = fminf(r, fminf(g, b));

        hsv out;
        out.v = max;

        if (max == 0.0f)             { out.s = 0.f; out.h = 0.f; return out; }
        if (max - min == 0.0f)       { out.s = 0.f; out.h = 0.f; return out; }

        out.s = (max - min) / max;

        if (max == r)      out.h =       (g - b) / (max - min);
        else if (max == g) out.h = 2.f + (b - r) / (max - min);
        else               out.h = 4.f + (r - g) / (max - min);

        out.h *= 60.f;
        if (out.h < 0.f) out.h += 360.f;
        return out;
    }

    inline void hsv_to_rgb(float h, float s, float v, int* r, int* g, int* b) {
        // Clamp rather than wrap: config.json is hand-editable and a typo should
        // produce a dull colour, not a random one.
        if (h < 0.f) h = 0.f;   if (h > 360.f) h = 360.f;
        if (s < 0.f) s = 0.f;   if (s > 1.f)   s = 1.f;
        if (v < 0.f) v = 0.f;   if (v > 1.f)   v = 1.f;

        float rf = v, gf = v, bf = v;
        if (s > 0.f) {
            float hh = (h >= 360.f ? 0.f : h) / 60.f;
            int   i  = (int)hh;
            float f  = hh - (float)i;
            float p  = v * (1.f - s);
            float q  = v * (1.f - s * f);
            float t  = v * (1.f - s * (1.f - f));
            switch (i) {
                case 0: rf = v; gf = t; bf = p; break;
                case 1: rf = q; gf = v; bf = p; break;
                case 2: rf = p; gf = v; bf = t; break;
                case 3: rf = p; gf = q; bf = v; break;
                case 4: rf = t; gf = p; bf = v; break;
                default:rf = v; gf = p; bf = q; break;
            }
        }

        *r = (int)(rf * 255.f + 0.5f);
        *g = (int)(gf * 255.f + 0.5f);
        *b = (int)(bf * 255.f + 0.5f);
    }
}
