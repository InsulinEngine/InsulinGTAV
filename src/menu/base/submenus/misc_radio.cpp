#include "menu/base/submenus/misc_radio.h"
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
    bool g_mobile_radio = false;
}

void misc_radio_menu::load() {
    set_name("Radio");
    set_parent<misc_menu>();

    add_option(toggle_option("Mobile Radio")
        .add_toggle(g_mobile_radio)
        .add_tooltip("Radio keeps playing on foot")
        .add_savable(get_submenu_name_stack()));

    add_option(button_option("Skip Radio Track")
        .add_click([] { native::skip_radio_forward(); }));
}

void misc_radio_menu::feature_update() {
    if (g_mobile_radio)
        native::set_mobile_radio_enabled_during_gameplay(true);
}

misc_radio_menu* misc_radio_menu::get() {
    static misc_radio_menu instance;
    return &instance;
}
