#pragma once

// Downscale targets. Free of the STL and of PS4 headers so the host test can
// include it, like frame_clock.h and camera_dir_math.h.
namespace util::image {

    // Fit src into the max box, aspect preserved, never enlarging. Both outputs
    // are clamped to at least 1: a zero dimension is not a small picture, it is
    // a crash in whatever allocates w*h*4 next.
    inline void fit_within(int src_w, int src_h, int max_w, int max_h,
                           int* out_w, int* out_h) {
        if (src_w < 1) src_w = 1;
        if (src_h < 1) src_h = 1;

        if (src_w <= max_w && src_h <= max_h) {
            *out_w = src_w;
            *out_h = src_h;
            return;
        }

        // Integer maths, and the wider ratio wins so both dimensions land inside.
        long long w = (long long)src_w * max_h;
        long long h = (long long)src_h * max_w;

        if (w > h) {              // width is the binding constraint
            *out_w = max_w;
            *out_h = (int)(((long long)src_h * max_w) / src_w);
        } else {
            *out_h = max_h;
            *out_w = (int)(((long long)src_w * max_h) / src_h);
        }

        if (*out_w < 1) *out_w = 1;
        if (*out_h < 1) *out_h = 1;
    }
}
