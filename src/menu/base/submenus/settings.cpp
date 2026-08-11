#include "menu/base/submenus/settings.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/theme.h"
#include "menu/base/util/notify.h"
#include "util/translation.h"
#include <stdio.h>

// Set by the Save-Theme click; the theme list is rebuilt in settings_menu::update()
// on the NEXT frame instead of re-entrantly from inside the click (which runs while
// update_menu is iterating the option list).
static bool g_themes_dirty = false;

void settings_menu::load() {
    set_name("Settings");
    set_parent<main_menu>();

    add_option(button_option("Save Theme")
        .add_tooltip("Save current colours / fonts / positions as theme_N.json (rename the file to taste)")
        .add_click([] {
            char name[32];
            snprintf(name, sizeof(name), "theme_%d", (int)menu::theme::list().size() + 1);
            menu::theme::save(name);
            menu::notify::stacked("Theme", "Saved");
            g_themes_dirty = true;   // list refreshed safely in update(), not re-entrantly here
        }));

    add_option(button_option("Reset to Default")
        .add_tooltip("Restore the built-in default theme")
        .add_click([] { menu::theme::reset_to_default(); menu::notify::stacked("Theme", "Reset to default"); }));

    add_option(submenu_option("Language")
        .add_submenu<language_menu>()
        .add_tooltip("Switch UI language (from /data/insulin/lang)"));

    add_option(break_option("Themes").ref());

    update_once();
}

void settings_menu::update() {
    // Rebuild the theme list a frame after a save (safe: update() runs at the top of
    // update_menu, before the option list is iterated).
    if (g_themes_dirty) { g_themes_dirty = false; update_once(); }
}

void settings_menu::update_once() {
    clear_options(4);   // keep Save Theme / Reset / Language / break

    stl::vector<stl::string> themes = menu::theme::list();
    if (themes.size() == 0) {
        add_option(button_option("~m~(no themes saved)").ref());
        return;
    }
    for (size_t i = 0; i < themes.size(); i++) {
        int idx = (int)i;   // capture a tiny int (stl::function caps captures at 64B; stl::string is 128B)
        add_option(button_option(themes[i])
            .add_tooltip("Apply this theme")
            .add_click([idx] {
                stl::vector<stl::string> list = menu::theme::list();
                if (idx >= 0 && idx < (int)list.size()) {
                    menu::theme::load_by_name(list[idx].c_str());
                    menu::notify::stacked("Theme", "Applied");
                }
            }));
    }
}

settings_menu* settings_menu::get() {
    static settings_menu instance;
    return &instance;
}

// ---- language submenu -------------------------------------------------------
void language_menu::load() {
    set_name("Language");
    set_parent<settings_menu>();

    add_option(button_option("English (Default)")
        .add_tooltip("Reset to the original English strings")
        .add_click([] { util::i18n::reset(); menu::notify::stacked("Language", "English"); }));

    add_option(break_option("Languages").ref());

    update_once();
}

void language_menu::update_once() {
    clear_options(2);   // keep English + break

    stl::vector<stl::string> langs = util::i18n::list();
    if (langs.size() == 0) {
        add_option(button_option("~m~(no language files)").ref());
        return;
    }
    for (size_t i = 0; i < langs.size(); i++) {
        int idx = (int)i;   // tiny capture (stl::function caps captures at 64B)
        add_option(button_option(langs[i])
            .add_tooltip("Apply this language")
            .add_click([idx] {
                stl::vector<stl::string> list = util::i18n::list();
                if (idx >= 0 && idx < (int)list.size()) {
                    util::i18n::load_by_name(list[idx].c_str());
                    menu::notify::stacked("Language", "Applied");
                }
            }));
    }
}

language_menu* language_menu::get() {
    static language_menu instance;
    return &instance;
}
