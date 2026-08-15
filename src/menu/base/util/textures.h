#pragma once
#include "platform/stdafx.h"

// Texture registry: the names menu_texture slots may resolve to. Ozark loaded
// custom .png assets into a grc texture dictionary (WIC on PC); this port's
// equivalent is rage::gfx::texture_dictionary ("insulin"), fed by
// menu::images::convert()/apply(). load() lists the stems available in
// /data/Ozark/images (menu::images::list_sources()) so renderer::get_texture
// can find a picked name by name, not by index.
namespace menu::textures {
    struct texture_context {
        stl::string m_name;
    };

    class textures {
    public:
        void load();
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
