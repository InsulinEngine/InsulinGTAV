#include "menu/base/submenus/misc_camera.h"
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
    bool g_first_person_off = false;
}

void misc_camera_menu::load() {
    set_name("Camera");
    set_parent<misc_menu>();

    add_option(toggle_option("Disable First Person")
        .add_toggle(g_first_person_off)
        .add_tooltip("Keeps the game out of the first-person camera")
        .add_savable(get_submenu_name_stack()));
}

void misc_camera_menu::feature_update() {
    // No blanket "disable first person" native on this build. The cinematic
    // director is what actually switches the view, so that is what gets blocked.
    if (g_first_person_off)
        native::invalidate_idle_cam();
}

misc_camera_menu* misc_camera_menu::get() {
    static misc_camera_menu instance;
    return &instance;
}
