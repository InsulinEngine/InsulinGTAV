#include "menu/base/submenus/misc_disables.h"
#include "menu/base/submenus/misc.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"

namespace {
    bool g_no_phone = false;
    bool g_no_minimap = false;
    bool g_no_cinematic = false;
    bool g_minimap_latched = false;
}

void misc_disables_menu::load() {
    set_name("Disables");
    set_parent<misc_menu>();

    add_option(toggle_option("Disable Phone")
        .add_toggle(g_no_phone).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Minimap")
        .add_toggle(g_no_minimap).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Disable Cinematic Camera")
        .add_toggle(g_no_cinematic).add_savable(get_submenu_name_stack()));
}

void misc_disables_menu::feature_update() {
    if (g_no_phone)
        native::disable_control_action(0, 27, true);

    if (g_no_cinematic)
        native::set_cinematic_button_active(false);

    if (g_no_minimap) {
        native::display_radar(false);
        g_minimap_latched = true;
    } else if (g_minimap_latched) {
        native::display_radar(true);
        g_minimap_latched = false;
    }
}

misc_disables_menu* misc_disables_menu::get() {
    static misc_disables_menu instance;
    return &instance;
}
