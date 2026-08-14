#include "menu/base/submenus/misc_visions.h"
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
    bool g_thermal = false;
    bool g_night = false;
    bool g_thermal_latched = false;
    bool g_night_latched = false;
}

void misc_visions_menu::load() {
    set_name("Visions");
    set_parent<misc_menu>();

    add_option(toggle_option("Thermal Vision")
        .add_toggle(g_thermal).add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Night Vision")
        .add_toggle(g_night).add_savable(get_submenu_name_stack()));
}

void misc_visions_menu::feature_update() {
    // Both persist, so each is switched rather than pushed every frame.
    if (g_thermal != g_thermal_latched) {
        native::set_seethrough(g_thermal);
        g_thermal_latched = g_thermal;
    }
    if (g_night != g_night_latched) {
        native::set_nightvision(g_night);
        g_night_latched = g_night;
    }
}

misc_visions_menu* misc_visions_menu::get() {
    static misc_visions_menu instance;
    return &instance;
}
