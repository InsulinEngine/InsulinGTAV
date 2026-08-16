#pragma once
#include <stdint.h>
#include "stl/vector.h"

// Runtime custom-texture support for GTA V PS4 (CUSA00411 v1.57).
//
// Loads image files (DDS/PNG) via RAGE's own grcImage loader + grcTextureFactory,
// then injects them into the game texture-dictionary store so the ordinary script
// native draw_sprite(dict, name, ...) resolves them -- no custom present hook, no
// .otd authoring. RVAs + evidence: memory/insulingtav-custom-textures.md.
//
// Threading: create/commit touch the RAGE GPU allocator + txd store; call from the
// script/game thread (menu tick or a button), with a session (e.g. Story Mode) up.
namespace rage::gfx {
    // Create a grcTexture from an image file at `path` (absolute, e.g.
    // "/data/Ozark/logo.dds"). Passing NULL params is safe. Returns the
    // grcTexture* or nullptr. Prefer texture_dictionary below for drawing.
    void* create_texture_from_file(const char* path);

    // A named texture dictionary backed by the game's txd store. Every texture
    // added and committed becomes drawable via draw_sprite(name(), <tex name>).
    class texture_dictionary {
    public:
        explicit texture_dictionary(const char* dict_name);

        // Load `path` and add it under `tex_name`. If `tex_name` is null/empty the
        // lowercased file stem is used ("/data/Ozark/logo.dds" -> "logo").
        // Buffers the texture; call commit() to (re)inject. Re-adding a name
        // replaces it. Returns true if the texture was created.
        // DDS only -- the engine's image loader parses no other format, and hands
        // back a magenta/green checkerboard for anything else (see gfx.cpp).
        bool add(const char* tex_name, const char* path);

        // Add an already-created grcTexture under `tex_name`.
        bool add_texture(const char* tex_name, void* tex);

        // Scan `dir` for *.dds / *.png and add each (name = lowercased file stem).
        // Returns the number added. Call commit() afterwards. .png is still scanned
        // on purpose: add() rejects it with a named error, which beats a stray PNG
        // being silently skipped when someone wonders why their texture is missing.
        int add_directory(const char* dir);

        // (Re)build the pgDictionary from the buffered textures and inject/refresh
        // it in the txd store. Safe to call repeatedly. Returns true on success.
        bool commit();

        bool has(const char* tex_name) const;

        // The grcTexture already registered under `tex_name`, or null. Lets a
        // caller give a second name to a texture that is already loaded instead
        // of loading the same file twice.
        void* get(const char* tex_name) const;

        int  count() const;
        bool ready() const;
        const char* name() const;   // dictionary name, for draw_sprite()

    private:
        struct entry { uint32_t code; void* tex; char name[64]; };
        stl::vector<entry> m_entries;
        char m_dict[64];
        int  m_slot;
        bool m_committed;
        bool m_injected;
    };

    // The menu's shared custom dictionary (name "insulin").
    texture_dictionary& menu_textures();

    // True if `dict_name` was injected by this manager -- the renderer must not
    // try to stream/request it (it lives only in memory).
    bool is_custom_dict(const char* dict_name);

    // Sample the txd store slot the last commit() installed into: report any
    // change the engine makes to it, and otherwise emit one heartbeat a minute
    // carrying a monotonic frame count. Reads only -- no natives, no writes --
    // so it is safe to call every frame from the game thread, and it is inert
    // until something has been committed. The heartbeat is what dates a fault:
    // this plugin's crashes end the log without a line of their own.
    void watch_store_slot();

    // The watcher's monotonic frame count. Exposed so other per-frame work can
    // pace itself off the same clock the log is timestamped with, which keeps a
    // log line and the thing it describes on one timeline.
    uint32_t watch_frame();

    // Convenience for the header: menu_textures() committed and holds "logo".
    bool banner_ready();
}
