#include "menu/base/submenus/helper_esp_settings_edit.h"
#include "menu/base/submenus/helper_esp_settings.h"
#include "menu/base/submenus/helper_color.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/button.h"
#include "menu/base/util/rainbow.h"

namespace {
    color_rgba* g_color   = nullptr;
    bool*       g_rainbow = nullptr;
    const char* g_title   = "Colour";
    color_rgba* g_built_for = nullptr;
}

void helper_esp_settings_edit_menu::target(color_rgba* colour, bool* rainbow, const char* title) {
    g_color   = colour;
    g_rainbow = rainbow;
    g_title   = title ? title : "Colour";
}

void helper_esp_settings_edit_menu::load() {
    set_name("Colour");
    set_parent<helper_esp_settings_menu>();
    update_once();
}

void helper_esp_settings_edit_menu::update_once() {
    clear_options(0);
    g_built_for = g_color;

    if (!g_color || !g_rainbow) {
        add_option(button_option("~m~Nothing selected").ref());
        return;
    }
    set_name(g_title);

    // helper_color_menu::target has no revert path (Task 6) - it takes the
    // colour pointer and a title only.
    add_option(submenu_option("Colour")
        .add_submenu<helper_color_menu>()
        .add_click([] {
            if (!g_color) return;
            // helper_color_menu is shared, and its load() points its parent at
            // Settings > Themes because Themes was its only opener. Its own
            // header says so in writing: a second opener has to re-point it,
            // or "back" out of an ESP colour lands in Themes. Themes re-points
            // it the same way from its side.
            helper_color_menu::get()->set_parent<helper_esp_settings_edit_menu>();
            helper_color_menu::target(g_color, g_title);
        }));

    // Ozark gives every element its own animator object. This port already has
    // a shared one with a registry, so the toggle registers and unregisters the
    // colour instead - same behaviour, and menu::tick already runs the pass.
    add_option(toggle_option("Rainbow")
        .add_toggle(*g_rainbow)
        .add_click([] {
            if (!g_color || !g_rainbow) return;
            // toggle_option flips *g_rainbow before calling this handler, so act
            // on its new value. m_enabled is what actually starts the animator -
            // rainbow::run() early-returns without it - and it is off until
            // something switches it on, so registering a colour without setting
            // it reads as "on" and never cycles. Same as helper_color.cpp, down
            // to leaving m_enabled alone on the way out: removing the last
            // colour makes run() a no-op by itself (m_colors is empty), and
            // clearing the flag would only make the next add() forget to set it.
            menu::rainbow* rb = menu::get_rainbow();
            if (*g_rainbow) { rb->add(g_color); rb->m_enabled = true; }
            else              rb->remove(g_color);
        })
        .add_tooltip("Cycles this colour through the hue wheel"));
}

void helper_esp_settings_edit_menu::update() {
    if (g_built_for != g_color) update_once();
}

helper_esp_settings_edit_menu* helper_esp_settings_edit_menu::get() {
    static helper_esp_settings_edit_menu instance;
    return &instance;
}
