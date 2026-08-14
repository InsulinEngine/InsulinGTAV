#include "menu/base/submenus/helper_color_presets.h"
#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/renderer.h"

namespace {
    struct preset { const char* m_name; color_rgba m_color; };

    // Constant-initialised: color_rgba's constructors are constexpr, which is
    // what lets this sit at namespace scope without a global constructor.
    const preset g_presets[] = {
        { "Ozark Blue",     color_rgba(0x00, 0x95, 0xFF, 255) },
        { "Emerald",        color_rgba(0x00, 0x9B, 0x77, 200) },
        { "Tangerine",      color_rgba(0xDD, 0x41, 0x24, 200) },
        { "Honeysuckle",    color_rgba(0xD6, 0x50, 0x76, 200) },
        { "Turquoise",      color_rgba(0x44, 0xB8, 0xAC, 200) },
        { "Mimosa",         color_rgba(0xEF, 0xC0, 0x50, 200) },
        { "Chili Pepper",   color_rgba(0x9B, 0x23, 0x35, 200) },
        { "True Red",       color_rgba(0xBC, 0x24, 0x3C, 200) },
        { "Cerulean",       color_rgba(0x98, 0xB4, 0xD4, 200) },
        { "Galaxy Blue",    color_rgba(0x2A, 0x4B, 0x7C, 200) },
        { "Orange Tiger",   color_rgba(0xF9, 0x67, 0x14, 200) },
        { "Pink Peacock",   color_rgba(0xC6, 0x21, 0x68, 200) },
        { "Aspen Gold",     color_rgba(0xFF, 0xD6, 0x62, 200) },
        { "Eclipse",        color_rgba(0x34, 0x31, 0x48, 200) },
        { "Nebulas Blue",   color_rgba(0x3F, 0x69, 0xAA, 200) },
        { "Quetzal Green",  color_rgba(0x00, 0x6E, 0x6D, 200) },
        { "Baby Blue",      color_rgba(0x6F, 0x9F, 0xD8, 200) },
        { "Ultra Violet",   color_rgba(0x6B, 0x5B, 0x95, 200) },
        { "Lime Punch",     color_rgba(0xBF, 0xD6, 0x41, 200) },
        { "Harbor Mist",    color_rgba(0xB4, 0xB7, 0xBA, 200) },
    };
    const int g_preset_count = (int)(sizeof(g_presets) / sizeof(g_presets[0]));
}

void helper_color_presets_menu::load() {
    set_name("Presets");

    for (int i = 0; i < g_preset_count; i++) {
        add_option(button_option(g_presets[i].m_name)
            .add_click([i] {
                color_rgba* t = helper_color_menu::target_color();
                if (t) { int a = t->a; *t = g_presets[i].m_color; t->a = a; }
                menu::submenu::handler::set_submenu_previous(false);
            })
            .add_hover([i] { menu::renderer::render_color_preview(g_presets[i].m_color); }));
    }
}

helper_color_presets_menu* helper_color_presets_menu::get() {
    static helper_color_presets_menu instance;
    return &instance;
}
