#include "menu/base/submenus/helper_color_sync.h"
#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/renderer.h"
#include "menu/base/util/theme.h"

void helper_color_sync_menu::load() {
    set_name("Sync With...");

    // One button per registry entry - this is the surface that could not exist
    // before the registry, because nothing else could enumerate the colours.
    // color_display_name(), not color_name(): the label is user-facing, and
    // color_name() is the raw config key ("option_selected") that saved theme
    // files depend on.
    for (int i = 0; i < menu::theme::color_count(); i++) {
        add_option(button_option(menu::theme::color_display_name(i))
            .add_click([i] {
                color_rgba* t = helper_color_menu::target_color();
                color_rgba* s = menu::theme::color_ptr(i);
                if (t && s && t != s) *t = *s;
                menu::submenu::handler::set_submenu_previous(false);
            })
            .add_hover([i] {
                menu::renderer::render_color_preview(*menu::theme::color_ptr(i));
            }));
    }
}

helper_color_sync_menu* helper_color_sync_menu::get() {
    static helper_color_sync_menu instance;
    return &instance;
}
