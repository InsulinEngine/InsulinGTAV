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

    // Is there already a cache entry for this stem, sized for this slot? The
    // cache is keyed by (slot, name): header and background cache the same
    // source at different boxes, so a name alone would let one slot's cache
    // answer for the other and hand back a stretched frame.
    bool is_cached(const char* name, slot s);

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
