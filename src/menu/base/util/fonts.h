#pragma once
#include "platform/stdafx.h"

// Font handling, trimmed for the PS4 base. Ozark's fonts::load scanned .gfx
// files from disk and registered them through the game's font store; there is
// no custom-font pipeline in scope here, so load()/update() are no-ops and
// get_font_id maps the handful of names the base uses to built-in font ids.
namespace menu::fonts {
    struct font_context {
        stl::string m_font_name;
        int m_font_id = -1;
        int m_asset_id = -1;
    };

    class fonts {
    public:
        void load() {}
        void update() {}
        void update_queue() {}
        int get_font_id(stl::string name);

        stl::vector<font_context>& get_list() { return m_fonts; }
    private:
        stl::vector<font_context> m_fonts;
    };

    fonts* get_fonts();

    inline void load() { get_fonts()->load(); }
    inline void update() { get_fonts()->update(); }
    inline void update_queue() { get_fonts()->update_queue(); }
    inline int get_font_id(stl::string name) { return get_fonts()->get_font_id(name); }
    inline stl::vector<font_context>& get_list() { return get_fonts()->get_list(); }
}
