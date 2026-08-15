#include "menu/base/submenus/settings.h"
#include "menu/base/submenus/settings_streamer.h"
#include "menu/base/submenus/settings_themes.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/notify.h"
#include "util/translation.h"

void settings_menu::load() {
    set_name("Settings");
    set_parent<main_menu>();

    add_option(submenu_option("Themes").add_submenu<settings_themes_menu>());
    add_option(submenu_option("Streamer Mode").add_submenu<settings_streamer_menu>());

    add_option(submenu_option("Language")
        .add_submenu<language_menu>()
        .add_tooltip("Switch UI language (from /data/Ozark/lang)"));
}

void settings_menu::update() {
}

void settings_menu::update_once() {
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
