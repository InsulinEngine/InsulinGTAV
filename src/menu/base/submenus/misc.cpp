#include "menu/base/submenus/misc.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/misc_camera.h"
#include "menu/base/submenus/misc_radio.h"
#include "menu/base/submenus/misc_visions.h"
#include "menu/base/submenus/misc_disables.h"
#include "menu/base/submenus/misc_dispatch.h"
#include "menu/base/submenus/misc_panels.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/render_parts.h"
#include "global/ui_vars.h"
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

    // DIAGNOSTICS - bisecting the phone background. One switch per drawn part;
    // turn them off one at a time and watch which one takes the artefact with it.
    add_option(toggle_option("[draw] Header")
        .add_toggle(menu::parts::g_header));
    add_option(toggle_option("[draw] Background")
        .add_toggle(menu::parts::g_background));
    add_option(toggle_option("[draw] Scroller")
        .add_toggle(menu::parts::g_scroller));
    add_option(toggle_option("[draw] Footer")
        .add_toggle(menu::parts::g_footer));
    add_option(toggle_option("[draw] Scrollbar")
        .add_toggle(menu::parts::g_scrollbar));
    add_option(toggle_option("[draw] Option Counter")
        .add_toggle(menu::parts::g_counter));
    add_option(toggle_option("[draw] Instructional Bar")
        .add_toggle(menu::parts::g_instructionals));
    add_option(toggle_option("[draw] Panels")
        .add_toggle(menu::parts::g_panels));


    add_option(submenu_option("Radio").add_submenu<misc_radio_menu>());
    add_option(submenu_option("Visions").add_submenu<misc_visions_menu>());
    add_option(submenu_option("Disables").add_submenu<misc_disables_menu>());
    add_option(submenu_option("Dispatch").add_submenu<misc_dispatch_menu>());
    add_option(submenu_option("Panels").add_submenu<misc_panels_menu>());

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
