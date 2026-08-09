#include "menu/menu.h"
#include "menu/base/base.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "global/ui_vars.h"
#include "rage/invoker/natives.h"

namespace menu {
    void build() {
        // Bind the string/pointer-bearing texture globals (skipped by the absent
        // .init_array), then set up the submenu tree.
        global::ui::init();
        menu::submenu::handler::load();
    }

    void tick() {
        // g_delta drives the scroller lerp; refresh it from the frame time.
        global::ui::g_delta = native::get_frame_time();

        // Input first (open bind L1+O + navigation), then base (control-disable +
        // render + handler update), then drain any deferred menu-input actions.
        menu::input::update();
        menu::base::update();
        menu::input::mi_update();
    }
}
