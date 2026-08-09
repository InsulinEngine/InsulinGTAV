#include "menu/base/util/fonts.h"

namespace menu::fonts {
    // Built-in RAGE font ids for the names the base references. Anything else
    // falls back to font 0 (the default HUD font).
    int fonts::get_font_id(stl::string name) {
        if (name == "RDR") return 7;   // matches Ozark's g_header_font usage
        return 0;
    }

    fonts* get_fonts() {
        static fonts instance;
        return &instance;
    }
}
