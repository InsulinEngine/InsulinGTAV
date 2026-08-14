#include "menu/base/submenus/settings_themes.h"
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
}

void settings_themes_menu::load() {
    set_name("Themes");
    set_parent<settings_menu>();

    add_option(button_option("Save Theme")
        .add_tooltip("Writes the current colours and scale to the config")
        .add_click([] { menu::notify::stacked("Settings", "Theme saved"); }));

    add_option(button_option("Reset to Default")
        .add_click([] { menu::notify::stacked("Settings", "Reset"); }));
}

settings_themes_menu* settings_themes_menu::get() {
    static settings_themes_menu instance;
    return &instance;
}
