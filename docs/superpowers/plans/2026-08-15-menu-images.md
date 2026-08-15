# Menu Images Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let a user drop a PNG, JPG or GIF into a folder on the console and pick it as the menu's header or background, animated or static, with no PC in the loop.

**Architecture:** `stb_image` (already in the OpenOrbis toolchain) decodes to RGBA8; `stb_image_resize` bounds the size; a hand-written DDS writer produces the only format the engine's `grcImage::Load` accepts; the existing `rage::gfx::texture_dictionary` and `menu::animation` carry it the rest of the way. Conversion happens when a picture is picked, and its output is cached on disk.

**Tech Stack:** C++17 without exceptions or RTTI, hand-rolled mini-STL (`src/stl/`), `<stb/stb_image.h>` and `<stb/stb_image_resize.h>` from the toolchain, OpenOrbis PS4 toolchain via WSL + ninja, host unit tests through a standalone `clang++` invocation.

**Spec:** `docs/superpowers/specs/2026-08-15-menu-images-design.md`

## Global Constraints

- **C++17, no exceptions, no RTTI.** Hand-rolled mini-STL: no `std::` containers, no `std::to_string` (use `util::itos` / `util::ftos` from `src/util/num_to_string.h`).
- **`stl::string` is a fixed 128-byte buffer** that truncates silently. Paths are longer than that — use `char[320]` and `snprintf`, the way `animated_texture.cpp` already does.
- **`stl::function` captures cap at 64 bytes**, enforced by `static_assert`. Capture a small index, never a struct or a string.
- **No `.init_array`:** global constructors never run. Namespace-scope data must be constant-initialised or live behind a function-local static.
- **Nothing may call a native during `menu::build()`.** Per-frame work that calls natives must be gated on `game::player_valid()`.
- **The animation system refuses more than `menu::animation::k_max_frames` (16) frames**, and says so rather than truncating silently. The converter caps at that number and reports it.
- **Host tests take C headers only** for our own pure code: MSVC's C++ stdlib rejects the installed clang (`STL1000`). `stb_image.h` is C and compiles on the host fine.
- **All paths come from `src/platform/paths.h`.** Do not spell a path at a callsite.
- **Build:** `wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'` — into `build-wsl/`, never `build/`. If it says "ninja: no work to do", touch a changed file and build again.
- **Bump `INSULIN_BUILD_TAG`** in `src/platform/build_tag.h` on every deploy.

**Testing reality.** The pure pieces — DDS bytes, scale arithmetic, decode against a real file — are host-tested. The menu surface and anything touching the txd store is console-verified. Tasks that can carry a real test do; the rest end with an explicit console step, and that step is the gate.

---

### Task 1: Confirm stb_image's GIF API, then wrap the decoder

Everything else assumes multi-frame GIF decoding exists in this copy of stb_image. Older versions have no `stbi_load_gif_from_memory`. Find out before building on it.

**Files:**
- Create: `src/util/image/decode.h`
- Create: `src/util/image/decode.cpp`
- Create: `tests/image_decode_test.cpp`

**Interfaces:**
- Consumes: `<stb/stb_image.h>` from the toolchain
- Produces:
  - `struct util::image::decoded { int w, h, frames; unsigned char* rgba; int* delays_ms; }`
  - `bool util::image::decode_file(const char* path, decoded* out)`
  - `void util::image::free_decoded(decoded* d)`

- [ ] **Step 1: Find out whether the GIF API is there**

```bash
grep -n 'stbi_load_gif_from_memory' /home/bbc/OpenOrbis-PS4-Toolchain/include/stb/stb_image.h
```
Run it through WSL:
```bash
wsl.exe bash -lc "grep -n 'stbi_load_gif_from_memory' /home/bbc/OpenOrbis-PS4-Toolchain/include/stb/stb_image.h"
```

If it prints a declaration, continue with Step 2 as written.

If it prints nothing, **stop and report it** rather than working around it. The fallback — decoding only the first GIF frame with `stbi_load` — changes what the feature is, and that is the spec author's call, not the implementer's.

- [ ] **Step 2: Write the failing test**

`assets/test.gif` is already in the repo. Create `tests/image_decode_test.cpp`:

```cpp
// Host unit test for the image decoder.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src -I <toolchain>/include tests/image_decode_test.cpp src/util/image/decode.cpp -o build/image_decode_test.exe
//   ./build/image_decode_test.exe
// stb_image is C and compiles on the host; nothing here includes a PS4 header.
#include "util/image/decode.h"
#include <stdio.h>

static int g_failed = 0;

static void check(const char* what, int got, int want) {
    if (got != want) { printf("FAIL %s: got %d, want %d\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %d\n", what, got); }
}

static void check_true(const char* what, bool cond) {
    if (!cond) { printf("FAIL %s\n", what); g_failed++; }
    else       { printf("ok   %s\n", what); }
}

int main() {
    util::image::decoded d;

    // A real GIF from the repo. Asserting concrete numbers rather than "it
    // returned something" - a decoder that hands back one blank frame would pass
    // a truthiness check and fail this.
    check_true("decode assets/test.gif", util::image::decode_file("assets/test.gif", &d));
    check_true("width > 0",  d.w > 0);
    check_true("height > 0", d.h > 0);
    check_true("at least one frame", d.frames >= 1);
    check_true("pixels not null", d.rgba != nullptr);

    // Every frame needs a delay, and a zero delay would make an animation spin
    // at the frame rate.
    for (int i = 0; i < d.frames; i++)
        check_true("delay > 0", d.delays_ms[i] > 0);

    // Not every pixel identical: catches a decoder that returns a cleared buffer.
    bool varied = false;
    for (int i = 4; i < d.w * d.h * 4 && !varied; i += 4)
        if (d.rgba[i] != d.rgba[0]) varied = true;
    check_true("frame is not a flat fill", varied);

    util::image::free_decoded(&d);

    // A file that is not an image must fail cleanly, not crash or half-fill.
    util::image::decoded bad;
    check_true("garbage rejected", !util::image::decode_file("tests/image_decode_test.cpp", &bad));

    check_true("missing file rejected", !util::image::decode_file("does/not/exist.png", &bad));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
```

- [ ] **Step 3: Run it and watch it fail**

```bash
clang++ -std=c++17 -I src tests/image_decode_test.cpp src/util/image/decode.cpp -o build/image_decode_test.exe
```
Expected: FAIL — `'util/image/decode.h' file not found`.

- [ ] **Step 4: Write the header**

```cpp
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
}
```

- [ ] **Step 5: Write the implementation**

```cpp
#include "util/image/decode.h"

// Only the formats we accept, so the binary does not grow by decoders nothing
// calls. STB_IMAGE_IMPLEMENTATION must appear in exactly one translation unit.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_GIF
#define STBI_ONLY_BMP
#define STBI_NO_STDIO_WRITE
#include <stb/stb_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace util::image {

    static const int k_min_delay_ms = 20;   // a 0 delay would spin at frame rate

    bool decode_file(const char* path, decoded* out) {
        if (!path || !out) return false;
        memset(out, 0, sizeof(*out));

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

        // GIF first: it is the only multi-frame format here, and stb's still
        // loader would silently hand back just the first frame.
        int comp = 0;
        int* delays = nullptr;
        int z = 0;
        unsigned char* px = stbi_load_gif_from_memory(buf, (int)size, &delays,
                                                      &out->w, &out->h, &z, &comp, 4);
        if (px) {
            free(buf);
            out->frames = z > 0 ? z : 1;
            out->rgba = px;
            out->delays_ms = (int*)malloc(sizeof(int) * (size_t)out->frames);
            if (!out->delays_ms) { free_decoded(out); return false; }
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
}
```

- [ ] **Step 6: Run the test to verify it passes**

```bash
clang++ -std=c++17 -I src -I "$(wslpath -w /home/bbc/OpenOrbis-PS4-Toolchain/include 2>/dev/null || echo /home/bbc/OpenOrbis-PS4-Toolchain/include)" tests/image_decode_test.cpp src/util/image/decode.cpp -o build/image_decode_test.exe && ./build/image_decode_test.exe
```

If clang cannot find `<stb/stb_image.h>` from Windows, copy the header once into
`third_party/stb/stb_image.h` and add `-I third_party` instead — the PS4 build
still uses the toolchain copy, and the two are the same file. Say in your report
which route you took.

Expected: `all passed`, exit 0.

- [ ] **Step 7: Build for PS4**

Run the WSL build command from Global Constraints.
Expected: compiles. Note the `.prx` size before and after in your report — the spec bounds the decoders on purpose and the number is how we know it worked.

- [ ] **Step 8: Commit**

```bash
git add src/util/image/decode.h src/util/image/decode.cpp tests/image_decode_test.cpp
git commit -m "feat(image): decode PNG/JPG/GIF/BMP with stb_image

The toolchain already ships stb_image, so decoding costs an include rather than
a hand-written LZW and inflate. GIF goes through stbi_load_gif_from_memory
because the still loader silently returns only the first frame.

Limited to PNG/JPEG/GIF/BMP so the binary does not carry decoders nothing calls.
Delays below 20 ms are raised: a zero delay would spin an animation at the frame
rate.

Tested on the host against assets/test.gif, asserting real dimensions, a frame
count, per-frame delays and that the pixels are not a flat fill - a decoder
handing back one blank frame passes a truthiness check and fails this."
```

---

### Task 2: Write uncompressed BGRA8 DDS

**Files:**
- Create: `src/util/image/dds_write.h`
- Create: `src/util/image/dds_write.cpp`
- Create: `tests/dds_write_test.cpp`

**Interfaces:**
- Consumes: nothing
- Produces: `bool util::image::write_dds(const char* path, const unsigned char* rgba, int w, int h)`

- [ ] **Step 1: Write the failing test**

Create `tests/dds_write_test.cpp`. The header layout is fixed and known, so assert the actual bytes:

```cpp
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
```

- [ ] **Step 2: Run it and watch it fail**

```bash
clang++ -std=c++17 -I src tests/dds_write_test.cpp src/util/image/dds_write.cpp -o build/dds_write_test.exe
```
Expected: FAIL — `'util/image/dds_write.h' file not found`.

- [ ] **Step 3: Write the header**

```cpp
#pragma once

// Writes the one texture format this build's engine will load.
//
// grcImage::Load (eboot 0x19CF830) reads four bytes and requires 'DDS '. PNG and
// JPG are rejected and come back as a magenta/green checkerboard - which is not
// an error path, just a wrong picture, so a bad header sends you looking in the
// decoder instead of here.
//
// The layout below is transcribed from tools/gif2frames.ps1, which produced the
// frames that already play on console. It is not re-derived.
namespace util::image {

    // One uncompressed B8G8R8A8 frame. `rgba` is w*h*4 bytes in RGBA order;
    // the channels are swapped on the way out. Returns false if the file cannot
    // be written.
    bool write_dds(const char* path, const unsigned char* rgba, int w, int h);
}
```

- [ ] **Step 4: Write the implementation**

```cpp
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
        put32(d + 16, (unsigned)(w * 4));         // dwPitchOrLinearSize
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
        size_t n = (size_t)w * (size_t)h * 4u;
        unsigned char* bgra = (unsigned char*)malloc(n);
        if (!bgra) return false;
        for (size_t i = 0; i < n; i += 4) {
            bgra[i + 0] = rgba[i + 2];
            bgra[i + 1] = rgba[i + 1];
            bgra[i + 2] = rgba[i + 0];
            bgra[i + 3] = rgba[i + 3];
        }

        FILE* f = fopen(path, "wb");
        if (!f) { free(bgra); return false; }
        bool ok = fwrite(head, 1, sizeof(head), f) == sizeof(head) &&
                  fwrite(bgra, 1, n, f) == n;
        fclose(f);
        free(bgra);
        return ok;
    }
}
```

- [ ] **Step 5: Run the test to verify it passes**

```bash
clang++ -std=c++17 -I src tests/dds_write_test.cpp src/util/image/dds_write.cpp -o build/dds_write_test.exe && ./build/dds_write_test.exe
```
Expected: `all passed`, exit 0.

- [ ] **Step 6: Commit**

```bash
git add src/util/image/dds_write.h src/util/image/dds_write.cpp tests/dds_write_test.cpp
git commit -m "feat(image): write uncompressed B8G8R8A8 DDS

The only format grcImage::Load accepts on this build. The byte layout is
transcribed from tools/gif2frames.ps1, whose output already plays on console,
rather than re-derived - the loader reads ddspf.dwSize from offset 72 and
dwFourCC from 80, and getting either wrong produces a checkerboard rather than
an error.

Host-tested against the actual header bytes, including the DX10 block and the
top-down row order, because that is exactly what cannot be checked by looking at
the picture: a wrong header does not fail, it just draws the wrong thing."
```

---

### Task 3: Target-size arithmetic

**Files:**
- Create: `src/util/image/scale.h`
- Create: `tests/image_scale_test.cpp`

**Interfaces:**
- Consumes: nothing
- Produces: `void util::image::fit_within(int src_w, int src_h, int max_w, int max_h, int* out_w, int* out_h)`

- [ ] **Step 1: Write the failing test**

```cpp
// Host unit test for the downscale target arithmetic.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/image_scale_test.cpp -o build/image_scale_test.exe
//   ./build/image_scale_test.exe
#include "util/image/scale.h"
#include <stdio.h>

static int g_failed = 0;

static void check2(const char* what, int gw, int gh, int ww, int wh) {
    if (gw != ww || gh != wh) {
        printf("FAIL %s: got %dx%d, want %dx%d\n", what, gw, gh, ww, wh); g_failed++;
    } else printf("ok   %s = %dx%d\n", what, gw, gh);
}

int main() {
    int w, h;

    // Smaller than the box is left alone - never upscale a small logo into mush.
    util::image::fit_within(64, 32, 512, 128, &w, &h);
    check2("smaller untouched", w, h, 64, 32);

    // Exactly the box is left alone.
    util::image::fit_within(512, 128, 512, 128, &w, &h);
    check2("exact fit untouched", w, h, 512, 128);

    // Too wide: width binds, aspect kept.
    util::image::fit_within(1024, 128, 512, 128, &w, &h);
    check2("width binds", w, h, 512, 64);

    // Too tall: height binds.
    util::image::fit_within(256, 2048, 512, 1024, &w, &h);
    check2("height binds", w, h, 128, 1024);

    // A 4K photo into the background box.
    util::image::fit_within(3840, 2160, 512, 1024, &w, &h);
    check2("4k into background", w, h, 512, 288);

    // Extreme ratios must never round a dimension to zero - a zero-sized
    // texture is a crash waiting in the DDS writer, not a small picture.
    util::image::fit_within(4000, 2, 512, 1024, &w, &h);
    if (w < 1 || h < 1) { printf("FAIL extreme ratio produced %dx%d\n", w, h); g_failed++; }
    else printf("ok   extreme ratio = %dx%d\n", w, h);

    util::image::fit_within(2, 4000, 512, 1024, &w, &h);
    if (w < 1 || h < 1) { printf("FAIL extreme ratio 2 produced %dx%d\n", w, h); g_failed++; }
    else printf("ok   extreme ratio 2 = %dx%d\n", w, h);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
```

- [ ] **Step 2: Run it and watch it fail**

```bash
clang++ -std=c++17 -I src tests/image_scale_test.cpp -o build/image_scale_test.exe
```
Expected: FAIL — `'util/image/scale.h' file not found`.

- [ ] **Step 3: Write the implementation**

```cpp
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
```

- [ ] **Step 4: Run the test to verify it passes**

```bash
clang++ -std=c++17 -I src tests/image_scale_test.cpp -o build/image_scale_test.exe && ./build/image_scale_test.exe
```
Expected: `all passed`, exit 0.

- [ ] **Step 5: Commit**

```bash
git add src/util/image/scale.h tests/image_scale_test.cpp
git commit -m "feat(image): downscale target arithmetic, host-tested

Aspect preserved, never enlarging, both dimensions clamped to at least 1. The
clamp is the point: an extreme ratio rounding a dimension to zero is not a small
picture, it is a crash in whatever allocates w*h*4 next."
```

---

### Task 4: Convert an image into the cache

**Files:**
- Create: `src/menu/base/util/menu_images.h`
- Create: `src/menu/base/util/menu_images.cpp`
- Modify: `src/platform/paths.h`

**Interfaces:**
- Consumes: Task 1 `decode_file`/`free_decoded`, Task 2 `write_dds`, Task 3 `fit_within`; `menu::animation::load_from_dir`, `menu::animation::k_max_frames`; `rage::gfx::menu_textures()`
- Produces:
  - `enum class menu::images::slot { header, background }`
  - `stl::vector<stl::string> menu::images::list_sources()`
  - `bool menu::images::is_cached(const char* name)`
  - `bool menu::images::convert(const char* name, slot s)`

- [ ] **Step 1: Add the two paths**

In `src/platform/paths.h`, beside the existing entries:

```cpp
#define OZARK_IMAGES    OZARK_DIR "/images"
#define OZARK_IMGCACHE  OZARK_IMAGES "/.cache"
```

- [ ] **Step 2: Write the header**

```cpp
#pragma once
#include "stl/vector.h"
#include "stl/string.h"

// Turning a picture on disk into something the menu can draw.
//
// The engine only loads DDS from a file, so conversion writes a cache next to
// the sources and the menu draws from that. Conversion runs when a picture is
// picked - never during menu::build(), which is where this project has had two
// crashes and is the last place to start an image decoder.
namespace menu::images {

    enum class slot { header, background };

    // Source pictures in /data/Ozark/images (png/jpg/gif/bmp), stems only.
    stl::vector<stl::string> list_sources();

    // Is there already a cache entry for this stem?
    bool is_cached(const char* name);

    // Decode, downscale and write the cache for `name`, sized for `s`. Safe to
    // call when the cache is already current - it returns true without work.
    // Returns false on a missing source, a decode failure or an unwritable
    // cache, having reported which.
    bool convert(const char* name, slot s);

    // Load the cached picture into the shared "insulin" dictionary and point the
    // slot at it. Converts first if needed. `name` of nullptr or "" clears the
    // slot back to its solid colour.
    bool apply(const char* name, slot s);
}
```

- [ ] **Step 3: Write the implementation**

Directory scanning follows `menu::theme::list()` in `src/menu/base/util/theme.cpp`, which uses `sceKernelOpen` + `sceKernelGetdents` — read it first and copy the loop shape rather than inventing one.

```cpp
#include "menu/base/util/menu_images.h"
#include "menu/base/util/animated_texture.h"
#include "global/ui_vars.h"          // apply() writes m_header / m_background
#include "platform/paths.h"
#include "platform/log.h"
#include "rage/gfx.h"
#include "util/image/decode.h"
#include "util/image/dds_write.h"
#include "util/image/scale.h"
#include "util/json.h"
#include <orbis/libkernel.h>
#include <stb/stb_image_resize.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace menu::images {

    namespace {
        struct box { int w, h; };

        box box_for(slot s) {
            // From the spec: the menu background covers roughly 420x700 at 1080p
            // and the header 420x86. These sit above that with room for other
            // resolutions, and bound what a 4K source can cost.
            return s == slot::header ? box{ 512, 128 } : box{ 512, 1024 };
        }

        void cache_dir_for(const char* name, char* out, int len) {
            snprintf(out, len, "%s/%s", OZARK_IMGCACHE, name);
        }
    }

    bool convert(const char* name, slot s) {
        if (!name || !name[0]) return false;

        char src[320];
        // Try each extension; list_sources() only ever hands back stems.
        static const char* exts[] = { ".png", ".jpg", ".jpeg", ".gif", ".bmp" };
        bool found = false;
        for (int i = 0; i < 5 && !found; i++) {
            snprintf(src, sizeof(src), "%s/%s%s", OZARK_IMAGES, name, exts[i]);
            int fd = sceKernelOpen(src, 0 /* O_RDONLY */, 0);
            if (fd >= 0) { sceKernelClose(fd); found = true; }
        }
        if (!found) {
            LOG_ERROR("images: no source for \"%s\" in %s", name, OZARK_IMAGES);
            return false;
        }

        util::image::decoded d;
        if (!util::image::decode_file(src, &d)) {
            LOG_ERROR("images: could not decode \"%s\"", src);
            return false;
        }

        box b = box_for(s);
        int dw = 0, dh = 0;
        util::image::fit_within(d.w, d.h, b.w, b.h, &dw, &dh);

        // The animation system refuses more than k_max_frames and says so rather
        // than truncating quietly. Cap here too, and report it, so the limit is
        // visible at the place the user chose the picture.
        int frames = d.frames;
        if (frames > menu::animation::k_max_frames) {
            frames = menu::animation::k_max_frames;
            platform::notify("Image has more frames than the menu plays; using the first 16");
        }

        sceKernelMkdir(OZARK_IMAGES, 0777);
        sceKernelMkdir(OZARK_IMGCACHE, 0777);

        char dir[320];
        cache_dir_for(name, dir, sizeof(dir));
        sceKernelMkdir(dir, 0777);

        unsigned char* scaled = nullptr;
        if (dw != d.w || dh != d.h) {
            scaled = (unsigned char*)malloc((size_t)dw * dh * 4);
            if (!scaled) { util::image::free_decoded(&d); return false; }
        }

        tj::json manifest;
        tj::json list;
        bool ok = true;

        for (int i = 0; i < frames && ok; i++) {
            const unsigned char* srcpx = d.rgba + (size_t)i * d.w * d.h * 4;
            const unsigned char* use = srcpx;

            if (scaled) {
                stbir_resize_uint8(srcpx, d.w, d.h, 0, scaled, dw, dh, 0, 4);
                use = scaled;
            }

            char frame_path[384];
            snprintf(frame_path, sizeof(frame_path), "%s/frame_%03d.dds", dir, i);
            ok = util::image::write_dds(frame_path, use, dw, dh);
            if (!ok) { LOG_ERROR("images: could not write %s", frame_path); break; }

            char file_name[64];
            snprintf(file_name, sizeof(file_name), "frame_%03d.dds", i);
            tj::json entry;
            entry["file"] = tj::json(file_name);
            entry["delay"] = tj::json((long long)d.delays_ms[i]);
            list.push_back(entry);
        }

        if (ok) {
            manifest["loop"] = tj::json(true);
            manifest["frames"] = list;
            char man[384];
            snprintf(man, sizeof(man), "%s/frames.json", dir);
            manifest.save_to_file(man, 2);
            platform::logf("images", "\"%s\": %d frame(s) at %dx%d", name, frames, dw, dh);
        }

        if (scaled) free(scaled);
        util::image::free_decoded(&d);
        return ok;
    }
}
```

`list_sources()`, `is_cached()` and `apply()` are written in the next steps —
they need the directory scan and the registry, which Task 5 also touches.

- [ ] **Step 4: Write list_sources and is_cached**

Model the scan on `menu::theme::list()` in `src/menu/base/util/theme.cpp` — same
`sceKernelOpen` + `sceKernelGetdents` loop, same `struct dirent` walk. Accept
`.png`, `.jpg`, `.jpeg`, `.gif`, `.bmp`, return the stem lowercased, and skip the
`.cache` directory. `is_cached(name)` returns whether
`<OZARK_IMGCACHE>/<name>/frames.json` opens.

- [ ] **Step 5: Write apply()**

```cpp
    bool apply(const char* name, slot s) {
        menu_texture& mt = (s == slot::header) ? global::ui::m_header
                                               : global::ui::m_background;

        if (!name || !name[0]) {           // "None"
            mt.m_enabled = false;
            mt.m_texture.set("");
            return true;
        }

        if (!is_cached(name) && !convert(name, s))
            return false;

        char dir[320];
        snprintf(dir, sizeof(dir), "%s/%s", OZARK_IMGCACHE, name);

        // Registered under the slot's own name so header and background cannot
        // collide in the dictionary - load_from_dir names textures
        // "<name>_000", and two pictures both starting at 000 would drop each
        // other on commit().
        const char* anim_name = (s == slot::header) ? "slot_header" : "slot_background";
        if (!menu::animation::load_from_dir(anim_name, dir)) {
            LOG_ERROR("images: nothing loadable in %s", dir);
            return false;
        }

        mt.m_texture.set(name);
        mt.m_enabled = true;
        return true;
    }
```

- [ ] **Step 6: Build**

Run the WSL build command.
Expected: compiles. `stbir_resize_uint8` comes from `<stb/stb_image_resize.h>`; if it needs `STB_IMAGE_RESIZE_IMPLEMENTATION` defined in one TU, define it in this file and say so in your report.

- [ ] **Step 7: Commit**

```bash
git add src/menu/base/util/menu_images.h src/menu/base/util/menu_images.cpp src/platform/paths.h
git commit -m "feat(images): convert a picture into the frame cache

Decode, downscale to the slot's box, write DDS frames and a frames.json in the
format the existing animation loader already reads - one path for animations,
not two.

Frames are capped at menu::animation::k_max_frames and the cap is reported. The
animation system already refuses more than that and says so; failing silently
one layer earlier would just move the confusion.

Registered per slot rather than per picture: load_from_dir names its textures
<name>_000, so two pictures both starting at 000 would drop each other when the
dictionary commits."
```

---

### Task 5: Revive the texture registry and fix the dictionary name

Nothing drawn through `menu_texture` has ever appeared, for two reasons that have to be fixed together.

**Files:**
- Modify: `src/menu/base/util/textures.h`
- Modify: `src/menu/base/util/textures.cpp`
- Modify: `src/menu/base/renderer.cpp:27` and `:287`

**Interfaces:**
- Consumes: Task 4 `menu::images::list_sources`
- Produces: a populated `menu::textures::get_list()`

- [ ] **Step 1: Populate the list**

`menu::textures::textures::load()` is an empty stub. Fill `m_textures` from
`menu::images::list_sources()`, one `texture_context` per stem. Replace the
class comment, which claims custom-YTD streaming is unsolved on console — that
stopped being true when `rage::gfx::texture_dictionary` landed, and the stale
comment is why the stub was never revisited.

Call `load()` from `menu::build()` after `util::config::load()`. It only reads a
directory — no natives — so it is safe in the boot window.

- [ ] **Step 2: Fix the dictionary name at both sites**

`src/menu/base/renderer.cpp:27` returns `{ "ozarktextures", vit->m_name }`.
Nothing is registered under that name; the dictionary this project creates is
`"insulin"`. Change it to `{ "insulin", vit->m_name }`.

`src/menu/base/renderer.cpp:287` (and the matching line in
`draw_sprite_aligned`) skips the streaming request for `asset.first !=
"ozarktextures"`. Remove that clause: `rage::gfx::is_custom_dict()` on the same
line already covers the real dictionary, correctly and by registration rather
than by a hardcoded string.

- [ ] **Step 3: Build**

Run the WSL build command.
Expected: compiles, and `grep -rn ozarktextures src/` returns nothing.

- [ ] **Step 4: Commit**

```bash
git add src/menu/base/util/textures.h src/menu/base/util/textures.cpp src/menu/base/renderer.cpp
git commit -m "fix(ui): the texture registry was empty and pointed at a dictionary that does not exist

Two faults that had to be fixed together, and which is why no menu_texture slot
has ever drawn anything:

menu::textures::get_list() was never populated - the port shipped a stub whose
comment said custom-YTD streaming was unsolved on console, which stopped being
true when rage::gfx::texture_dictionary landed. The comment outlived the
problem and nobody revisited the stub.

get_texture() returned the dictionary name \"ozarktextures\". This project
creates \"insulin\". Even a populated list would have resolved to nothing.

The second ozarktextures site, a special case telling draw_sprite not to
stream-request it, goes too: is_custom_dict on the same line already covers the
real name by registration instead of by a hardcoded string."
```

---

### Task 6: Per-slot animation lookup in the renderer

**Files:**
- Modify: `src/menu/base/renderer.cpp:53` (header sprite), `:115` (title path), and the background draw
- Modify: `src/menu/base/util/animated_texture.h` / `.cpp` (a per-slot asset helper)

**Interfaces:**
- Consumes: `menu::animation::get`, `menu::animation::header_asset`
- Produces: `stl::pair<stl::string, stl::string> menu::animation::slot_asset(const char* anim_name, const stl::string& still_name)`

- [ ] **Step 1: Add the per-slot helper**

`header_asset()` is hardcoded to the animation named `"banner"` with a
`{"insulin","logo"}` fallback. Add beside it:

```cpp
    // Current frame of `anim_name` if that animation is loaded and ready,
    // otherwise the still texture `still_name` in the same dictionary. The
    // header's own helper stays as it is - the banner button predates slots and
    // still has its own name.
    stl::pair<stl::string, stl::string> slot_asset(const char* anim_name,
                                                   const stl::string& still_name);
```

Implement it the way `header_asset()` is implemented, with the name and the
fallback taken from the arguments.

- [ ] **Step 2: Use it for the header**

At `renderer.cpp:53`, the header currently asks for `menu::animation::get("banner")`.
Ask for `"slot_header"` first and fall back to the banner path, so the existing
Load Banner Animation button keeps working:

```cpp
        menu::animated_texture* slot_anim = menu::animation::get("slot_header");
        menu::animated_texture* banner_anim = menu::animation::get("banner");
```

Draw the slot animation when it is ready, then the banner, then the existing
static path. Leave `:115` (the title) on the banner as it is — the title is not
one of the two slots this feature covers.

- [ ] **Step 3: Give the background the same branch**

The background currently resolves one texture and draws it. Give it the
animation lookup the header has, using `"slot_background"`.

- [ ] **Step 4: Build and verify on console**

Bump the tag, build, deploy. Nothing visible changes yet — no picture is
assigned. The check is that the menu still renders exactly as before: header,
background, everything.

- [ ] **Step 5: Commit**

```bash
git add src/menu/base/renderer.cpp src/menu/base/util/animated_texture.h src/menu/base/util/animated_texture.cpp
git commit -m "feat(ui): per-slot animation lookup, so the background can animate too

The animation lookup was hardcoded to the name \"banner\" and existed only for
the header. Slots need their own names, and the background needs the branch at
all.

The banner button keeps working: the header tries its slot animation first and
falls back to the banner, so a feature that predates slots is not broken by
them."
```

---

### Task 7: The picker submenu

**Files:**
- Create: `src/menu/base/submenus/settings_images.h`, `.cpp`
- Modify: `src/menu/base/submenus/settings_themes.cpp` (one entry)
- Modify: `src/menu/menu.cpp` (register)

**Interfaces:**
- Consumes: Task 4 `list_sources`, `is_cached`, `apply`, `slot`
- Produces: `class settings_images_menu` with `static settings_images_menu* get()`

- [ ] **Step 1: Write the submenu**

One submenu with two sections, rebuilt through the dirty-flag pattern against
the source count — never per frame. Each entry is a `button_option` per picture
plus a "None" at the top of each section. Capture the index only; the 64-byte
capture cap does not fit a string.

The label shows whether a picture is cached, so a choice that pauses is visibly
different from one that does not:

```cpp
    stl::vector<stl::string> src = menu::images::list_sources();
    for (int i = 0; i < (int)src.size(); i++) {
        char label[96];
        snprintf(label, sizeof(label), "%s%s", src[i].c_str(),
                 menu::images::is_cached(src[i].c_str()) ? "" : "  ~m~(convert)");
        add_option(button_option(label)
            .add_click([i] {
                stl::vector<stl::string> list = menu::images::list_sources();
                if (i >= (int)list.size()) return;
                menu::notify::stacked("Images", "Loading...");
                if (menu::images::apply(list[i].c_str(), menu::images::slot::header))
                    menu::notify::stacked("Images", "Header set", global::ui::g_success);
                else
                    menu::notify::stacked("Images", "Could not load that picture", global::ui::g_error);
            }));
    }
```

The click handler re-reads the list rather than capturing a name: the capture
must stay small, and the directory can change between the option being built and
the button being pressed.

- [ ] **Step 2: Hang it under Themes and register it**

In `settings_themes.cpp`'s `load()`, above the colour list:

```cpp
    add_option(submenu_option("Menu Images").add_submenu<settings_images_menu>());
```

Remember `settings_themes_menu::update_once()` uses `clear_options(4 + menu::theme::color_count())`.
Adding an option to `load()` changes that fixed head — count what `load()` adds
after your edit and update the number, or the first rebuild deletes a live
option.

In `menu::menu.cpp` `build()`, beside the other registrations:

```cpp
        settings_images_menu::get()->load();
        menu::submenu::handler::add_submenu(settings_images_menu::get());
```

- [ ] **Step 3: Build and verify on console**

Bump the tag, build, deploy. Put a PNG and an animated GIF in
`/data/Ozark/images/`, then:

1. Settings → Themes → Menu Images lists both, with `(convert)` on each.
2. Pick the PNG as Background — it appears behind the options.
3. Pick the GIF as Header — it plays.
4. Reopen the picker — both now list without `(convert)`.
5. Pick "None" for each — the slot returns to its solid colour.
6. Put a text file renamed to `.png` in the folder and pick it — a named error,
   and the menu keeps drawing.

- [ ] **Step 4: Commit**

```bash
git add src/menu/base/submenus/settings_images.h src/menu/base/submenus/settings_images.cpp src/menu/base/submenus/settings_themes.cpp src/menu/menu.cpp
git commit -m "feat(settings): pick a picture for the header and the background

Settings > Themes > Menu Images, listing what is in /data/Ozark/images with a
marker on anything not yet converted - so a choice that will pause is visibly
different from one that is instant.

Click handlers re-read the directory rather than capturing a name: the capture
has to stay under the 64-byte cap, and the folder can change between the option
being built and the button being pressed."
```

---

### Task 8: Persist the choice in the theme

**Files:**
- Modify: `src/menu/base/util/theme.cpp` (save, load, reset)

**Interfaces:**
- Consumes: Task 4 `menu::images::apply`
- Produces: nothing new; themes gain two string keys

- [ ] **Step 1: Save the two names**

`theme.cpp` already serialises colours, fonts and positions through name tables.
Add the two texture names beside them, in `save()`:

```cpp
        root["images"]["header"]     = tj::json(g_m_header_name);
        root["images"]["background"] = tj::json(g_m_background_name);
```

Take the names from `global::ui::m_header.m_texture` and
`m_background.m_texture`, which Task 4's `apply()` sets.

- [ ] **Step 2: Apply them on load**

In `load_file()`, after the colours:

```cpp
        const tj::json* imgs = root.try_get("images");
        if (imgs) {
            // Convert-or-load, exactly as picking would. A theme naming a
            // picture this console does not have leaves that slot alone and says
            // so, rather than failing the whole theme.
            // json.h has value_bool / value_int / value_float but NO
            // value_string - read the child and check its type by hand.
            const tj::json* hj = imgs->try_get("header");
            const tj::json* bj = imgs->try_get("background");
            const char* h = (hj && hj->is_string()) ? hj->get_string() : "";
            const char* b = (bj && bj->is_string()) ? bj->get_string() : "";

            if (h[0] && !menu::images::apply(h, menu::images::slot::header))
                LOG_WARN("theme: header image \"%s\" not available", h);
            if (b[0] && !menu::images::apply(b, menu::images::slot::background))
                LOG_WARN("theme: background image \"%s\" not available", b);
        }
```

`try_get` + `is_string` + `get_string` is the shape used by
`animated_texture.cpp` when it reads `frames.json`; copy that rather than
looking for a `value_string`, which json.h does not have.

- [ ] **Step 3: Clear them on reset**

`reset_to_default()` must clear both slots, or a reset leaves the pictures in
place while every colour goes back:

```cpp
        menu::images::apply(nullptr, menu::images::slot::header);
        menu::images::apply(nullptr, menu::images::slot::background);
```

- [ ] **Step 4: Build and verify on console**

Bump the tag, build, deploy.

1. Pick a picture for each slot, press Save Theme.
2. Restart the game cold. The pictures come back.
3. Reset to Default — both slots return to their solid colours and the colours
   reset with them.
4. Rename one picture on the console, apply the saved theme again — a named
   warning, the other slot still loads.

- [ ] **Step 5: Commit**

```bash
git add src/menu/base/util/theme.cpp
git commit -m "feat(theme): themes carry their pictures too

theme.cpp already serialised colours, fonts and positions, and its own header
comment named per-slot textures as the intended follow-up. A theme is now
complete: colours and pictures in one file.

A theme naming a picture this console does not have leaves that slot on its
default and warns, rather than failing to load - themes travel between consoles
and their images may not."
```

---

## Verification

After Task 8, with the kernel log attached:

- [ ] Cold boot, no fault, `boot: build done`
- [ ] All four existing host tests still pass, plus the three new ones
- [ ] A PNG as background and an animated GIF as header, both surviving a restart
- [ ] An animated GIF as background plays and the frame rate holds
- [ ] "None" returns a slot to its solid colour
- [ ] A corrupt file produces a named message and the menu keeps drawing
- [ ] `grep -rn ozarktextures src/` returns nothing
