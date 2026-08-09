#include "menu/menu.h"
#include "menu/base/base.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/self.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "menu/base/util/control.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/stacked_display.h"
#include "menu/base/util/panels.h"
#include "util/config.h"
#include "global/ui_vars.h"
#include "rage/invoker/natives.h"

namespace menu {
    // Demo side-panel render callback (m_update is a plain function pointer).
    static math::vector2<float> demo_panel_update(menu::panels::panel_child& child) {
        menu::panels::panel p(child, global::ui::g_panel_bar);
        p.item("Health", "100");
        p.item("Armor", "50");
        p.item_full("Session", "Story Mode");
        return p.get_render_scale();
    }

    static void register_demo_panel() {
        panels::panel_parent* parent = new panels::panel_parent();
        parent->m_render = true;
        parent->m_id = "demo";
        parent->m_name = "Demo";

        panels::panel_child child{};
        child.m_parent = parent;
        child.m_render = true;
        child.m_id = "info";
        child.m_double_sided = true;
        child.m_panel_option_count_left = 3;
        child.m_panel_option_count_right = 0;
        child.m_update = demo_panel_update;

        parent->m_children_panels.push_back(child);
        panels::get_panels().push_back(parent);
    }

    void build() {
        // Bind the string/pointer-bearing texture globals (skipped by the absent
        // .init_array), load the config file, then set up the submenu tree and
        // populate the demo. Config is loaded BEFORE the submenus so each
        // add_savable() reads the persisted value.
        global::ui::init();
        util::config::load();
        menu::submenu::handler::load();   // m_current = main_menu::get()
        main_menu::get()->load();
        demo_child::get()->load();

        // Feature submenus: load + register so their feature_update runs each frame.
        self_menu::get()->load();
        menu::submenu::handler::add_submenu(self_menu::get());
        vehicle_menu::get()->load();
        menu::submenu::handler::add_submenu(vehicle_menu::get());

        register_demo_panel();
    }

    void tick() {
        // g_delta drives the scroller lerp; refresh it from the frame time.
        global::ui::g_delta = native::get_frame_time();

        // Input first (open bind L1+O + navigation), then base (control-disable +
        // render + handler update), then drain any deferred menu-input actions.
        menu::input::update();
        menu::base::update();
        menu::input::mi_update();

        // Per-frame feature loop (godmode etc. re-applied every frame, whether or
        // not the owning submenu is open), then the control manager's request
        // queues (model/asset streaming for spawns).
        menu::submenu::handler::feature_update();
        menu::control::update();

        // Notifications + stacked display + side panels render every frame
        // (panels::update no-ops while the menu is closed).
        menu::notify::update();
        menu::display::render();
        menu::panels::update();
    }
}
