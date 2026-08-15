#pragma once

// Image decoding via stb_image, which the OpenOrbis toolchain ships at
// include/stb/stb_image.h. Everything comes back as RGBA8.
//
// The engine cannot load PNG, JPG or GIF: grcImage::Load reads four bytes and
// demands 'DDS '. So this is only half the pipeline - decode here, then write
// DDS with util/image/dds_write.h and hand the engine a filename.
namespace util::image {

    struct decoded {
        int w;
        int h;
        int frames;                // 1 for a still image
        unsigned char* rgba;       // frames * w * h * 4, frame-major
        int* delays_ms;            // one per frame, never 0
    };

    // Decode any supported file. Returns false on a missing file, an unreadable
    // one, or a format stb_image does not handle - never half-fills `out`.
    bool decode_file(const char* path, decoded* out);

    // Release what decode_file allocated. Safe on a zeroed struct.
    void free_decoded(decoded* d);

    // Source dimensions without decoding pixels. False if the file is missing or
    // not an image this build understands.
    bool probe_file(const char* path, int* w, int* h);
}
