#pragma once
#include <stdint.h>

// Frame-selection maths for animated textures: which frame of a delay sequence
// is showing after N milliseconds.
//
// Deliberately free of STL, PS4 headers and engine state so it compiles for the
// host and is unit-tested on the PC (tests/frame_clock_test.cpp). It is the only
// part of the animation feature with real edge cases -- wrapping, one-shot
// endings, degenerate delay tables -- so it is the part worth testing off-target.
namespace menu::frame_clock {

    // Duration of one pass through `delays`, in ms. 0 if there is nothing to play.
    inline int total_ms(const uint16_t* delays, int count) {
        int total = 0;
        for (int i = 0; i < count; i++) total += (int)delays[i];
        return total;
    }

    // Index of the frame showing `elapsed_ms` into playback. A frame owns the
    // half-open span [start, start + delay).
    //   loop == true  -> the sequence repeats; elapsed wraps modulo the total
    //   loop == false -> playback stops on the last frame and stays there
    // Returns 0 for an empty sequence or an all-zero delay table (which would
    // otherwise divide by zero), and never returns an out-of-range index.
    inline int frame_at(const uint16_t* delays, int count, int elapsed_ms, bool loop) {
        if (count <= 0) return 0;
        const int total = total_ms(delays, count);
        if (total <= 0) return 0;
        if (elapsed_ms <= 0) return 0;

        if (elapsed_ms >= total) {
            if (!loop) return count - 1;
            elapsed_ms %= total;
        }

        int t = elapsed_ms;
        for (int i = 0; i < count; i++) {
            if (t < (int)delays[i]) return i;
            t -= (int)delays[i];
        }
        return count - 1;   // not reachable while total > 0; a guard, not a path
    }
}
