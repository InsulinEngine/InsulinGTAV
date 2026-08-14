#include "menu/base/submenus/settings_streamer.h"
#include "menu/base/submenus/settings.h"
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
    bool g_hide_names = false;
}

void settings_streamer_menu::load() {
    set_name("Streamer Mode");
    set_parent<settings_menu>();

    add_option(toggle_option("Hide Player Names")
        .add_toggle(g_hide_names)
        .add_tooltip("Session only - there are no other players in Story Mode")
        .add_savable(get_submenu_name_stack()));
}

settings_streamer_menu* settings_streamer_menu::get() {
    static settings_streamer_menu instance;
    return &instance;
}
