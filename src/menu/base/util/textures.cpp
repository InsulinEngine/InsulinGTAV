#include "menu/base/util/textures.h"
#include "menu/base/util/menu_images.h"

namespace menu::textures {
    // Cleared before refilling: this runs once at boot (menu::build(), after
    // util::config::load()) and again every time menu::images::apply()
    // succeeds, so a picture dropped in over FTP mid-session is findable by
    // name without doubling up entries from the previous scan.
    void textures::load() {
        m_textures.clear();
        for (stl::string& name : menu::images::list_sources()) {
            texture_context ctx;
            ctx.m_name = name;
            m_textures.push_back(ctx);
        }
    }

    textures* get_textures() {
        static textures instance;
        return &instance;
    }
}
