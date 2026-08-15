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
    // not an image this build understands. `frames` is optional; pass null if
    // you do not need it. It is counted out of the GIF's own block structure
    // (1 for every other format), because stb cannot report a frame count
    // without decoding every frame first - which is the allocation this exists
    // to let the caller avoid.
    bool probe_file(const char* path, int* w, int* h, int* frames = nullptr);

    // Bytes stb will have live at once decoding this source, worst case.
    //
    // A still image is one allocation of w*h*4. An animated GIF is not: stb
    // reallocs the buffer once per frame, so at the last growth the old and new
    // buffers are both live, and the peak is close to twice the final size.
    // That peak, not the final size, is what fails - and when it fails inside
    // stb's GIF path it does not return an error, it writes to a null pointer.
    unsigned long long decode_peak_bytes(int w, int h, int frames);

    // Size of the last allocation stb_image failed to make, or 0 if none has
    // failed since the last decode_file call. Reported this way, rather than
    // logged where it happens, so this module keeps depending on nothing but the
    // C library and stays buildable in the host tests.
    unsigned long long last_alloc_failure_bytes();
}
