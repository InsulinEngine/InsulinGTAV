#include "menu/base/submenus/misc.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/misc_camera.h"
#include "menu/base/submenus/misc_radio.h"
#include "menu/base/submenus/misc_visions.h"
#include "menu/base/submenus/misc_disables.h"
#include "menu/base/submenus/misc_dispatch.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"

// Ozark's Miscellaneous menu. ScriptHook, Stacked Display, Panels and Swaps are
// not here: they drive Ozark's own UI and script infrastructure rather than the
// game, so there is nothing to port until the equivalents exist.

namespace {
    bool g_hide_hud = false;
    bool g_hud_latched = false;
}

void misc_menu::load() {
    set_name("Miscellaneous");
    set_parent<main_menu>();

    add_option(submenu_option("Camera").add_submenu<misc_camera_menu>());
    add_option(submenu_option("Radio").add_submenu<misc_radio_menu>());
    add_option(submenu_option("Visions").add_submenu<misc_visions_menu>());
    add_option(submenu_option("Disables").add_submenu<misc_disables_menu>());
    add_option(submenu_option("Dispatch").add_submenu<misc_dispatch_menu>());

    add_option(break_option("HUD").ref());

    add_option(toggle_option("Hide HUD")
        .add_toggle(g_hide_hud)
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Clear Notifications")
        .add_click([] { native::thefeed_clear_frozen_post(); }));
}

void misc_menu::feature_update() {
    if (g_hide_hud) {
        native::display_hud(false);
        g_hud_latched = true;
    } else if (g_hud_latched) {
        native::display_hud(true);
        g_hud_latched = false;
    }
}

misc_menu* misc_menu::get() {
    static misc_menu instance;
    return &instance;
}
