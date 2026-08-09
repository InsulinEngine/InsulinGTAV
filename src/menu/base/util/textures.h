#pragma once
#include "platform/stdafx.h"

// Texture registry, stubbed for the PS4 base. Ozark loaded custom .png assets
// into a grc texture dictionary (WIC on PC); custom-YTD streaming is unsolved
// on console, so the base renders with the game's own dictionaries
// (commonmenu, ...) / sentinel quads. get_texture always misses, which is
// exactly what the renderer's fallback path expects. The DX11 texture pointer
// from the PC struct is dropped (not needed without a custom pipeline).
namespace menu::textures {
    struct texture_context {
        stl::string m_name;
    };

    class textures {
    public:
        void load() {}
        void update() {}
        bool get_texture(stl::string /*name*/, texture_context* /*out*/) { return false; }

        stl::vector<texture_context>& get_list() { return m_textures; }
    private:
        stl::vector<texture_context> m_textures;
    };

    textures* get_textures();

    inline void load() { get_textures()->load(); }
    inline void update() { get_textures()->update(); }
    inline bool get_texture(stl::string name, texture_context* out) { return get_textures()->get_texture(name, out); }
    inline stl::vector<texture_context>& get_list() { return get_textures()->get_list(); }
}
