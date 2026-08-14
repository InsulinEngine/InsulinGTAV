#pragma once

// Rainbow stepping, kept free of the STL and of every PS4 header so the host
// test can include it directly - the same split frame_clock.h uses against
// animated_texture.cpp.
//
// Hue advances 360/steps degrees per call and wraps. Saturation and value are
// full; the resulting channels are then remapped into [min,max] so the cycle
// never reaches black (unreadable menu text) or full saturation. Ozark's
// defaults - min 25, max 250, steps 80 - carry over as ours.
namespace menu::rainbow_math {

    struct rgb { int r, g, b; };

    inline rgb color_at(int step, int steps, int min, int max) {
        if (steps < 1) steps = 1;
        if (min > max) { int t = min; min = max; max = t; }
        if (min < 0) min = 0;
        if (max > 255) max = 255;

        // Hue in sixths, integer maths throughout: no <math.h>, no float drift
        // across a long-running cycle.
        int  wrapped = step % steps;
        if (wrapped < 0) wrapped += steps;

        int  h6      = (wrapped * 6) / steps;          // which sixth, 0..5
        int  within  = (wrapped * 6) % steps;          // position inside it
        int  rising  = (within * 255) / steps;         // 0..254
        int  falling = 255 - rising;

        int r = 0, g = 0, b = 0;
        switch (h6) {
            case 0: r = 255;     g = rising;  b = 0;       break;
            case 1: r = falling; g = 255;     b = 0;       break;
            case 2: r = 0;       g = 255;     b = rising;  break;
            case 3: r = 0;       g = falling; b = 255;     break;
            case 4: r = rising;  g = 0;       b = 255;     break;
            default:r = 255;     g = 0;       b = falling; break;
        }

        // Remap 0..255 into [min,max].
        int span = max - min;
        rgb out;
        out.r = min + (r * span) / 255;
        out.g = min + (g * span) / 255;
        out.b = min + (b * span) / 255;
        return out;
    }
}
