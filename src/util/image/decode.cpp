#include "util/image/decode.h"

// Only the formats we accept, so the binary does not grow by decoders nothing
// calls. STB_IMAGE_IMPLEMENTATION must appear in exactly one translation unit.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_GIF
#define STBI_ONLY_BMP
// SceLibcInternal does not export __assert_fail, so assert.h's default
// STBI_ASSERT(x) leaves the PS4 link with an undefined symbol. stb_image.h
// documents this override for exactly that situation.
#define STBI_ASSERT(x) ((void)0)

// stb's own allocators, replaced so an allocation failure is visible instead of
// fatal. This matters because stb's animated-GIF path does not check all of its
// allocations: in stbi__load_gif_main the first frame's buffer is taken with
//
//     out = (stbi_uc*)stbi__malloc( layers * stride );
//     ...
//     memcpy( out + ((layers - 1) * stride), u, stride );
//
// with no NULL test between them, and the per-frame `*delays` REALLOC is
// unchecked as well. On a desktop the allocation always succeeds and the gap
// never shows; in the game process it can fail, and then the memcpy writes to
// address 0 and takes the whole game down with no error anywhere.
//
// We compile STB_IMAGE_IMPLEMENTATION ourselves, so these macros are ours to
// set - the toolchain's stb_image.h is untouched, which matters because the PS4
// build resolves <stb/stb_image.h> from the toolchain, not from third_party/.
// The failing size is recorded rather than logged here, so this file keeps
// depending on nothing but the C library - that is what lets
// tests/image_decode_test.cpp build and run on the host against the same code
// the console runs. last_alloc_failure_bytes() hands the number to the caller,
// which is what turns "it died somewhere" into a budget chosen from evidence.
extern "C" void* util_image_stb_malloc(unsigned long long n);
extern "C" void* util_image_stb_realloc(void* p, unsigned long long n);
#define STBI_MALLOC(n)         util_image_stb_malloc((unsigned long long)(n))
#define STBI_REALLOC(p,n)      util_image_stb_realloc((p), (unsigned long long)(n))
#define STBI_FREE(p)           free(p)
#include <stb/stb_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long long g_last_alloc_failure = 0;

extern "C" void* util_image_stb_malloc(unsigned long long n) {
    void* p = malloc((size_t)n);
    if (!p) g_last_alloc_failure = n;
    return p;
}

extern "C" void* util_image_stb_realloc(void* prev, unsigned long long n) {
    void* p = realloc(prev, (size_t)n);
    if (!p) g_last_alloc_failure = n;
    return p;
}

namespace util::image {

    static const int k_min_delay_ms = 20;   // a 0 delay would spin at frame rate

    // Shared by decode_file and probe_file so there is exactly one place that
    // opens and reads a source file.
    static bool read_whole_file(const char* path, unsigned char** out_buf, long* out_size) {
        *out_buf = nullptr;
        *out_size = 0;

        FILE* f = fopen(path, "rb");
        if (!f) return false;
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (size <= 0) { fclose(f); return false; }

        unsigned char* buf = (unsigned char*)malloc((size_t)size);
        if (!buf) { fclose(f); return false; }
        size_t read = fread(buf, 1, (size_t)size, f);
        fclose(f);
        if (read != (size_t)size) { free(buf); return false; }

        *out_buf = buf;
        *out_size = size;
        return true;
    }

    bool decode_file(const char* path, decoded* out) {
        g_last_alloc_failure = 0;
        if (!path || !out) return false;
        memset(out, 0, sizeof(*out));

        unsigned char* buf = nullptr;
        long size = 0;
        if (!read_whole_file(path, &buf, &size)) return false;

        // GIF first: it is the only multi-frame format here, and stb's still
        // loader would silently hand back just the first frame.
        int comp = 0;
        int* delays = nullptr;
        int z = 0;
        unsigned char* px = stbi_load_gif_from_memory(buf, (int)size, &delays,
                                                      &out->w, &out->h, &z, &comp, 4);
        if (px) {
            free(buf);
            // z is documented as the frame count and stb only ever hands back
            // a null px when z == 0, so the ?: 1 below is currently dead - but
            // that is today's stb behaviour, not a contract, so the guard stays.
            out->frames = z > 0 ? z : 1;
            out->rgba = px;
            out->delays_ms = (int*)malloc(sizeof(int) * (size_t)out->frames);
            if (!out->delays_ms) {
                if (delays) STBI_FREE(delays);   // stb's buffer, before we bail
                free_decoded(out);
                return false;
            }
            for (int i = 0; i < out->frames; i++) {
                int d = delays ? delays[i] : 0;
                out->delays_ms[i] = d >= k_min_delay_ms ? d : k_min_delay_ms;
            }
            if (delays) STBI_FREE(delays);
            return true;
        }

        // Anything else: one frame.
        px = stbi_load_from_memory(buf, (int)size, &out->w, &out->h, &comp, 4);
        free(buf);
        if (!px) return false;

        out->frames = 1;
        out->rgba = px;
        out->delays_ms = (int*)malloc(sizeof(int));
        if (!out->delays_ms) { free_decoded(out); return false; }
        out->delays_ms[0] = k_min_delay_ms;
        return true;
    }

    void free_decoded(decoded* d) {
        if (!d) return;
        if (d->rgba) STBI_FREE(d->rgba);
        if (d->delays_ms) free(d->delays_ms);
        memset(d, 0, sizeof(*d));
    }

    // Count image descriptors in a GIF without decoding anything. The block
    // structure is walked, not guessed: 0x2C starts an image, 0x21 an extension
    // whose payload is a chain of length-prefixed sub-blocks, 0x3B ends the
    // file. Anything unexpected stops the walk and we return what was counted so
    // far - a wrong count here must never be worse than a conservative one.
    static int gif_frame_count(const unsigned char* d, long n) {
        if (n < 13 || d[0] != 'G' || d[1] != 'I' || d[2] != 'F') return 1;

        long i = 13;
        if (d[10] & 0x80) i += 3L * (1L << ((d[10] & 7) + 1));   // global colour table

        int frames = 0;
        while (i < n) {
            unsigned char b = d[i];
            if (b == 0x3B) break;                                // trailer
            if (b == 0x21) {                                     // extension
                if (i + 2 >= n) break;
                i += 2;
                while (i < n && d[i]) i += d[i] + 1;              // sub-block chain
                i += 1;
            } else if (b == 0x2C) {                              // image descriptor
                if (i + 10 > n) break;
                unsigned char lf = d[i + 9];
                i += 10;
                if (lf & 0x80) i += 3L * (1L << ((lf & 7) + 1));  // local colour table
                i += 1;                                           // LZW min code size
                while (i < n && d[i]) i += d[i] + 1;              // image data
                i += 1;
                frames++;
            } else {
                break;
            }
        }
        return frames > 0 ? frames : 1;
    }

    unsigned long long last_alloc_failure_bytes() { return g_last_alloc_failure; }

    unsigned long long decode_peak_bytes(int w, int h, int frames) {
        if (w < 1 || h < 1 || frames < 1) return 0;
        const unsigned long long stride = (unsigned long long)w * (unsigned long long)h * 4ull;
        const unsigned long long final_size = stride * (unsigned long long)frames;
        if (frames == 1) return final_size;
        // The last realloc holds the previous buffer (final - stride) and the
        // new one (final) at the same time.
        return final_size + (final_size - stride);
    }

    bool probe_file(const char* path, int* w, int* h, int* frames) {
        if (!path || !w || !h) return false;
        *w = 0; *h = 0;
        if (frames) *frames = 1;

        unsigned char* buf = nullptr;
        long size = 0;
        if (!read_whole_file(path, &buf, &size)) return false;

        int comp = 0;
        int ok = stbi_info_from_memory(buf, (int)size, w, h, &comp);
        if (ok && frames) *frames = gif_frame_count(buf, size);
        free(buf);
        return ok != 0;
    }
}
