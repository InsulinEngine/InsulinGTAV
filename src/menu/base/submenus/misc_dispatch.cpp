#include "menu/base/submenus/misc_dispatch.h"
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
    // The dispatch service ids the game uses, in the order Ozark lists them.
    struct svc { const char* name; int id; };
    const svc k_services[] = {
        { "Police Automobile",       1 },
        { "Police Helicopter",       2 },
        { "Police Riders",           4 },
        { "Police Vehicle Request",  5 },
        { "Police Road Block",       6 },
        { "Police Boat",            11 },
        { "Swat Automobile",         3 },
        { "Fire Department",         9 },
        { "Ambulance",              10 },
        { "Gangs",                   7 },
        { "Army Vehicle",           15 },
    };
    constexpr int SERVICE_COUNT = (int)(sizeof(k_services) / sizeof(k_services[0]));

    bool g_disabled[SERVICE_COUNT] = {};
}

void misc_dispatch_menu::load() {
    set_name("Dispatch");
    set_parent<misc_menu>();

    for (int i = 0; i < SERVICE_COUNT; i++) {
        add_option(toggle_option(k_services[i].name)
            .add_toggle(g_disabled[i])
            .add_savable(get_submenu_name_stack()));
    }
}

void misc_dispatch_menu::feature_update() {
    // ENABLE_DISPATCH_SERVICE is per-frame, so "off" simply means not disabling.
    for (int i = 0; i < SERVICE_COUNT; i++) {
        if (g_disabled[i])
            native::enable_dispatch_service(k_services[i].id, false);
    }
}

misc_dispatch_menu* misc_dispatch_menu::get() {
    static misc_dispatch_menu instance;
    return &instance;
}
