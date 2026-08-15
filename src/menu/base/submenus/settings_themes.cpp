#include "menu/base/submenus/settings_themes.h"
#include "menu/base/submenus/settings.h"
#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenus/settings_images.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/theme.h"
#include "menu/base/util/rainbow.h"
#include "menu/base/renderer.h"
#include "util/config.h"
#include <stdio.h>

namespace {
    // Starts true so the picker builds once on first show; the directory scan in
    // menu::theme::list() must not run unconditionally every frame (sceKernelOpen +
    // sceKernelGetdents), so update() only rebuilds when this flag is set.
    bool g_themes_dirty = true;

    // Config key (under this submenu's own name stack) remembering the last
    // theme applied, so build() can re-apply it at boot. Empty means "none" /
    // reset to default.
    const char* LAST_THEME_KEY = "LastTheme";
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
            util::config::write_string(settings_themes_menu::get()->get_submenu_name_stack(), LAST_THEME_KEY, name);
            menu::notify::stacked("Theme", "Saved");
            g_themes_dirty = true;   // list refreshed in update(), not re-entrantly here
        }));

    add_option(button_option("Reset to Default")
        .add_tooltip("Restore the built-in default theme")
        .add_click([] {
            // Before restoring: the rainbow holds pre-rainbow snapshots of the
            // colours it animates. Resetting underneath it would leave those
            // snapshots stale, and stopping later would undo the reset.
            menu::get_rainbow()->stop();
            menu::theme::reset_to_default();
            // Empty means "none" - otherwise the next boot would immediately
            // undo this reset by re-applying the last theme.
            util::config::write_string(settings_themes_menu::get()->get_submenu_name_stack(), LAST_THEME_KEY, "");
            menu::notify::stacked("Theme", "Reset to default");
        }));

    add_option(submenu_option("Menu Images").add_submenu<settings_images_menu>());

    add_option(break_option("Colours").ref());

    // Index only: stl::function caps captures at 64 bytes, and a color_entry
    // would not fit.
    for (int i = 0; i < menu::theme::color_count(); i++) {
        add_option(submenu_option(menu::theme::color_display_name(i))
            .add_submenu<helper_color_menu>()
            .add_click([i] { helper_color_menu::target(i); })
            .add_hover([i] (submenu_option*) {
                menu::renderer::render_color_preview(*menu::theme::color_ptr(i));
            }));
    }

    add_option(break_option("Saved Themes").ref());
}

void settings_themes_menu::update() {
    if (!g_themes_dirty) return;
    g_themes_dirty = false;
    update_once();
}

void settings_themes_menu::update_once() {
    stl::vector<stl::string> themes = menu::theme::list();
    // Static options from load(): Save Theme, Reset to Default, submenu_option
    // "Menu Images", break("Colours"), one submenu_option per registry colour,
    // then break("Saved Themes"). Only what follows that is rebuilt here.
    clear_options(5 + menu::theme::color_count());

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
                    // Same reasoning as Reset to Default: stop the rainbow first
                    // or it reverts this theme the moment it is next switched off.
                    menu::get_rainbow()->stop();
                    menu::theme::load_by_name(list[i].c_str());
                    util::config::write_string(settings_themes_menu::get()->get_submenu_name_stack(), LAST_THEME_KEY, list[i]);
                    menu::notify::stacked("Theme", "Applied");
                }
            }));
    }
}

settings_themes_menu* settings_themes_menu::get() {
    static settings_themes_menu instance;
    return &instance;
}

void settings_themes_menu::apply_last_theme() {
    stl::string last = util::config::read_string(get()->get_submenu_name_stack(), LAST_THEME_KEY, "");
    if (last.size() == 0) return;

    // theme::load_file/load_by_name touch files and platform::logf only - no
    // natives - so this is legal this early in build(). See theme.cpp.
    if (!menu::theme::load_by_name(last.c_str())) return;
}
