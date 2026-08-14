#include "menu/base/submenus/settings_themes.h"
#include "menu/base/submenus/settings.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/theme.h"
#include <stdio.h>

namespace {
    // Starts true so the picker builds once on first show; the directory scan in
    // menu::theme::list() must not run unconditionally every frame (sceKernelOpen +
    // sceKernelGetdents), so update() only rebuilds when this flag is set.
    bool g_themes_dirty = true;
}

void settings_themes_menu::load() {
    set_name("Themes");
    set_parent<settings_menu>();

    add_option(button_option("Save Theme")
        .add_tooltip("Save current colours / fonts / positions as theme_N.json (rename the file to taste)")
        .add_click([] {
            char name[32];
            snprintf(name, sizeof(name), "theme_%d", (int)menu::theme::list().size() + 1);
            menu::theme::save(name);
            menu::notify::stacked("Theme", "Saved");
            g_themes_dirty = true;   // list refreshed in update(), not re-entrantly here
        }));

    add_option(button_option("Reset to Default")
        .add_tooltip("Restore the built-in default theme")
        .add_click([] {
            menu::theme::reset_to_default();
            menu::notify::stacked("Theme", "Reset to default");
        }));

    add_option(break_option("Saved Themes").ref());
}

void settings_themes_menu::update() {
    if (!g_themes_dirty) return;
    g_themes_dirty = false;
    update_once();
}

void settings_themes_menu::update_once() {
    stl::vector<stl::string> themes = menu::theme::list();
    clear_options(3);

    if (themes.size() == 0) {
        add_option(button_option("~m~(no themes saved)").ref());
        return;
    }

    for (int i = 0; i < (int)themes.size(); i++) {
        add_option(button_option(themes[i])
            .add_tooltip("Apply this theme")
            .add_click([i] {
                stl::vector<stl::string> list = menu::theme::list();
                if (i < (int)list.size()) {
                    menu::theme::load_by_name(list[i].c_str());
                    menu::notify::stacked("Theme", "Applied");
                }
            }));
    }
}

settings_themes_menu* settings_themes_menu::get() {
    static settings_themes_menu instance;
    return &instance;
}
