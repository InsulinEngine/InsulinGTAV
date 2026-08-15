#include "util/image/dds_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace util::image {

    static void put32(unsigned char* p, unsigned v) {
        p[0] = (unsigned char)(v & 0xFF);
        p[1] = (unsigned char)((v >> 8) & 0xFF);
        p[2] = (unsigned char)((v >> 16) & 0xFF);
        p[3] = (unsigned char)((v >> 24) & 0xFF);
    }

    bool write_dds(const char* path, const unsigned char* rgba, int w, int h) {
        if (!path || !rgba || w <= 0 || h <= 0) return false;

        unsigned char head[4 + 124 + 20];
        memset(head, 0, sizeof(head));

        memcpy(head, "DDS ", 4);

        unsigned char* d = head + 4;              // DDS_HEADER
        put32(d + 0,  124);                       // dwSize
        put32(d + 4,  0x100F);                    // CAPS|HEIGHT|WIDTH|PITCH|PIXELFORMAT
        put32(d + 8,  (unsigned)h);
        put32(d + 12, (unsigned)w);
        put32(d + 16, (unsigned)((size_t)w * 4u)); // dwPitchOrLinearSize
        put32(d + 20, 0);                         // dwDepth
        put32(d + 24, 0);                         // dwMipMapCount - claiming more
                                                  // levels than the dimensions
                                                  // allow is one of the ways the
                                                  // loader bails to the checkerboard
        // dwReserved1[11] stays zero (offsets 28..71)

        put32(d + 72, 32);                        // ddspf.dwSize
        put32(d + 76, 4);                         // ddspf.dwFlags = DDPF_FOURCC
        memcpy(d + 80, "DX10", 4);                // ddspf.dwFourCC
        // bit count + RGBA masks stay zero (84..103)

        put32(d + 104, 0x1000);                   // dwCaps = DDSCAPS_TEXTURE
        // dwCaps2..4 + dwReserved2 stay zero (108..123)

        unsigned char* x = head + 4 + 124;        // DDS_HEADER_DXT10
        put32(x + 0,  87);                        // dxgiFormat = B8G8R8A8_UNORM
        put32(x + 4,  3);                         // resourceDimension = TEXTURE2D
        put32(x + 8,  0);                         // miscFlag
        put32(x + 12, 1);                         // arraySize - the loader multiplies by this
        put32(x + 16, 0);                         // miscFlags2

        // stb_image hands back RGBA; the format above is BGRA. Swap on the way
        // out rather than asking every caller to.
        //
        // One row at a time, not one whole image. This used to allocate a full
        // w*h*4 copy, which is a second image-sized block held at the same moment
        // as the decoded source and the scaled copy - and on this console that
        // third allocation is what failed first: a 1080x1920 JPG decoded fine and
        // then could not write its frame. A row buffer is 4*w, so the cost stops
        // scaling with height and the peak during a conversion drops by a whole
        // image.
        const size_t row_bytes = (size_t)w * 4u;
        unsigned char* row = (unsigned char*)malloc(row_bytes);
        if (!row) return false;

        FILE* f = fopen(path, "wb");
        if (!f) { free(row); return false; }

        bool ok = fwrite(head, 1, sizeof(head), f) == sizeof(head);
        for (int y = 0; ok && y < h; y++) {
            const unsigned char* src = rgba + (size_t)y * row_bytes;
            for (size_t i = 0; i < row_bytes; i += 4) {
                row[i + 0] = src[i + 2];
                row[i + 1] = src[i + 1];
                row[i + 2] = src[i + 0];
                row[i + 3] = src[i + 3];
            }
            ok = fwrite(row, 1, row_bytes, f) == row_bytes;
        }

        fclose(f);
        if (!ok) remove(path);
        free(row);
        return ok;
    }
}
