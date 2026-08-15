#include "util/image/decode.h"

// Only the formats we accept, so the binary does not grow by decoders nothing
// calls. STB_IMAGE_IMPLEMENTATION must appear in exactly one translation unit.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_GIF
#define STBI_ONLY_BMP
#define STBI_NO_STDIO_WRITE
// SceLibcInternal does not export __assert_fail, so assert.h's default
// STBI_ASSERT(x) leaves the PS4 link with an undefined symbol. stb_image.h
// documents this override for exactly that situation.
#define STBI_ASSERT(x) ((void)0)
#include <stb/stb_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    bool probe_file(const char* path, int* w, int* h) {
        if (!path || !w || !h) return false;
        *w = 0; *h = 0;

        unsigned char* buf = nullptr;
        long size = 0;
        if (!read_whole_file(path, &buf, &size)) return false;

        int comp = 0;
        int ok = stbi_info_from_memory(buf, (int)size, w, h, &comp);
        free(buf);
        return ok != 0;
    }
}
