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
            if (g_color) helper_color_menu::target(g_color, g_title);
        }));

    // Ozark gives every element its own animator object. This port already has
    // a shared one with a registry, so the toggle registers and unregisters the
    // colour instead - same behaviour, and menu::tick already runs the pass.
    add_option(toggle_option("Rainbow")
        .add_toggle(*g_rainbow)
        .add_click([] {
            if (!g_color || !g_rainbow) return;
            if (*g_rainbow) menu::get_rainbow()->add(g_color);
            else            menu::get_rainbow()->remove(g_color);
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
