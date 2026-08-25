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
