#include "menu/base/submenus/settings_images.h"
#include "menu/base/submenus/settings_themes.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/util/menu_images.h"
#include "menu/base/util/notify.h"
#include "global/ui_vars.h"
#include <stdio.h>

namespace {
    // Starts true so the picker builds once on first show; list_sources() is a
    // directory scan (sceKernelOpen + sceKernelGetdents) and is_cached() stats
    // both the cache and the source, so - same reasoning as
    // settings_themes.cpp's g_themes_dirty - update() must not redo that work
    // unconditionally every frame. Set true again after a successful apply() so
    // the "(convert)" marker this frame's rebuild (or the next time this
    // submenu is opened) drops for whatever just got cached.
    bool g_images_dirty = true;
}

void settings_images_menu::load() {
    set_name("Menu Images");
    set_parent<settings_themes_menu>();
}

void settings_images_menu::update() {
    if (!g_images_dirty) return;
    g_images_dirty = false;
    update_once();
}

void settings_images_menu::update_once() {
    // Nothing here is a static head - both sections are rebuilt from the
    // source list and each picture's cache state every time this runs.
    clear_options(0);

    stl::vector<stl::string> src = menu::images::list_sources();

    // ---- Header -------------------------------------------------------------
    add_option(break_option("Header").ref());

    add_option(button_option("None")
        .add_tooltip("Use the game's own header")
        .add_click([] {
            menu::images::apply(nullptr, menu::images::slot::header);
        }));

    for (int i = 0; i < (int)src.size(); i++) {
        char label[96];
        snprintf(label, sizeof(label), "%s%s", src[i].c_str(),
                 menu::images::is_cached(src[i].c_str(), menu::images::slot::header) ? "" : "  ~m~(convert)");
        add_option(button_option(label)
            .add_click([i] {
                // Index only: stl::function caps captures at 64 bytes, and the
                // directory can change between this option being built and the
                // button being pressed, so re-read rather than trust a stale list.
                stl::vector<stl::string> list = menu::images::list_sources();
                if (i >= (int)list.size()) return;
                menu::notify::stacked("Images", "Loading...");
                if (menu::images::apply(list[i].c_str(), menu::images::slot::header)) {
                    menu::notify::stacked("Images", "Header set", global::ui::g_success);
                    g_images_dirty = true;
                } else {
                    menu::notify::stacked("Images", "Could not load that picture", global::ui::g_error);
                }
            }));
    }

    // ---- Background -----------------------------------------------------------
    add_option(break_option("Background").ref());

    add_option(button_option("None")
        .add_tooltip("Use the game's own background")
        .add_click([] {
            menu::images::apply(nullptr, menu::images::slot::background);
        }));

    for (int i = 0; i < (int)src.size(); i++) {
        char label[96];
        snprintf(label, sizeof(label), "%s%s", src[i].c_str(),
                 menu::images::is_cached(src[i].c_str(), menu::images::slot::background) ? "" : "  ~m~(convert)");
        add_option(button_option(label)
            .add_click([i] {
                stl::vector<stl::string> list = menu::images::list_sources();
                if (i >= (int)list.size()) return;
                menu::notify::stacked("Images", "Loading...");
                if (menu::images::apply(list[i].c_str(), menu::images::slot::background)) {
                    menu::notify::stacked("Images", "Background set", global::ui::g_success);
                    g_images_dirty = true;
                } else {
                    menu::notify::stacked("Images", "Could not load that picture", global::ui::g_error);
                }
            }));
    }
}

settings_images_menu* settings_images_menu::get() {
    static settings_images_menu instance;
    return &instance;
}
