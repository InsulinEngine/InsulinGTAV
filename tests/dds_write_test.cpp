// Host unit test for the DDS writer.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/dds_write_test.cpp src/util/image/dds_write.cpp -o build/dds_write_test.exe
//   ./build/dds_write_test.exe
#include "util/image/dds_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failed = 0;

static void check(const char* what, unsigned got, unsigned want) {
    if (got != want) { printf("FAIL %s: got 0x%X, want 0x%X\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = 0x%X\n", what, got); }
}

static unsigned u32at(const unsigned char* b, int off) {
    return (unsigned)b[off] | ((unsigned)b[off+1] << 8) |
           ((unsigned)b[off+2] << 16) | ((unsigned)b[off+3] << 24);
}

int main() {
    const int w = 4, h = 2;
    unsigned char px[4 * 2 * 4];
    for (int i = 0; i < w * h * 4; i++) px[i] = (unsigned char)i;

    const char* path = "build/dds_write_test.dds";
    if (!util::image::write_dds(path, px, w, h)) {
        printf("FAIL write_dds returned false\n"); return 1;
    }

    FILE* f = fopen(path, "rb");
    if (!f) { printf("FAIL cannot reopen\n"); return 1; }
    fseek(f, 0, SEEK_END); long size = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char* b = (unsigned char*)malloc((size_t)size);
    fread(b, 1, (size_t)size, f);
    fclose(f);

    // 4 magic + 124 header + 20 DX10 + pixels
    check("file size", (unsigned)size, (unsigned)(4 + 124 + 20 + w * h * 4));

    check("magic", u32at(b, 0), 0x20534444u);          // 'DDS '
    check("dwSize", u32at(b, 4), 124u);
    check("dwFlags", u32at(b, 8), 0x100Fu);            // CAPS|HEIGHT|WIDTH|PITCH|PIXELFORMAT
    check("dwHeight", u32at(b, 12), (unsigned)h);
    check("dwWidth", u32at(b, 16), (unsigned)w);
    check("dwPitch", u32at(b, 20), (unsigned)(w * 4));
    check("dwMipMapCount", u32at(b, 28), 0u);

    // The loader reads ddspf.dwSize from header offset 72 and dwFourCC from 80,
    // counting from the start of the file. Those two are why a wrong layout
    // produces the checkerboard instead of an error.
    check("ddspf.dwSize", u32at(b, 4 + 72), 32u);
    check("ddspf.dwFlags", u32at(b, 4 + 76), 4u);      // DDPF_FOURCC
    check("ddspf.dwFourCC", u32at(b, 4 + 80), 0x30315844u);  // 'DX10'
    check("dwCaps", u32at(b, 4 + 104), 0x1000u);       // DDSCAPS_TEXTURE

    check("dxgiFormat", u32at(b, 4 + 124 + 0), 87u);   // B8G8R8A8_UNORM
    check("resourceDimension", u32at(b, 4 + 124 + 4), 3u);
    check("arraySize", u32at(b, 4 + 124 + 12), 1u);

    // Rows are top-down and pixels are copied verbatim.
    const int px_off = 4 + 124 + 20;
    // px[i] = i going in, and write_dds swaps RGBA to BGRA, so the first byte
    // out is px[2]. The last byte is the final alpha, which the swap leaves
    // alone. These are the values the swap produces - if they fail, the writer
    // and this test genuinely disagree, so fix the writer, not the numbers.
    check("first pixel byte (B of pixel 0)", b[px_off], 2u);
    check("last pixel byte (A of last pixel)", b[px_off + w * h * 4 - 1], (unsigned)(w * h * 4 - 1));

    free(b);
    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
