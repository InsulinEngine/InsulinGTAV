#include "menu/menu.h"
#include "menu/base/base.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/submenus/main.h"
#include "menu/base/submenus/self.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/submenus/settings.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "menu/base/util/control.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/stacked_display.h"
#include "menu/base/util/panels.h"
#include "menu/base/util/animated_texture.h"
#include "util/config.h"
#include "global/ui_vars.h"
#include "platform/system_ui.h"
#include "platform/log.h"
#include "rage/invoker/natives.h"

// Crash-tracing: while the menu is open, write one line per tick phase to
// /data/insulingtav.log. The last line on disk before a crash localises it:
//   - "overlaid-return" as the last line  -> guard fired; crash is OUTSIDE our
//     tick (the game touching our injected state) -> need the klog RIP.
//   - a phase name (input/base/render/...) as the last line -> crash is INSIDE
//     that phase of our code.
// Set to 0 for release builds.
#define INSULIN_TICK_TRACE 0

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
        settings_menu::get()->load();
        menu::submenu::handler::add_submenu(settings_menu::get());
        language_menu::get()->load();
        menu::submenu::handler::add_submenu(language_menu::get());

        register_demo_panel();
    }

    void tick() {
        bool open = menu::base::is_open();
        bool ov = platform::system_ui::overlaid();

#if INSULIN_TICK_TRACE
    // Dual sink so we capture the crash window no matter what: klog shows up live
    // in `nc <ip> 3232` (no FTP), and /data/insulingtav.log is the guaranteed
    // backup if plugin klog output doesn't reach the broadcast. Only while the
    // menu is open (the sole crash condition), so the volume stays bounded.
    #define TICK_TRACE(p) do { if (open) { \
            platform::klogf("tick:%s ov=%d", p, (int)ov); \
            platform::logf("tick", "%s ov=%d", p, (int)ov); \
        } } while (0)
#else
    #define TICK_TRACE(p) do { } while (0)
#endif

        TICK_TRACE("enter");

        // PS-button guard: while the ShellUI overlay (XMB) is up the game is
        // constrained and our per-frame native work crashes it. Skip the whole
        // tick - not just rendering - so no native is touched until the game
        // has the screen again. Features pause for the overlay's duration,
        // which is invisible: the game is paused under the overlay anyway.
        if (ov) { TICK_TRACE("overlaid-return"); return; }

        // g_delta drives the scroller lerp; refresh it from the frame time.
        global::ui::g_delta = native::get_frame_time();

        // Step every loaded animation before anything draws, so update and render
        // stay separate and the renderer keeps no side effects.
        menu::animation::update(global::ui::g_delta);

        // Input first (open bind L1+O + navigation), then base (control-disable +
        // render + handler update), then drain any deferred menu-input actions.
        TICK_TRACE("input");
        menu::input::update();
        TICK_TRACE("base");
        menu::base::update();
        TICK_TRACE("mi_update");
        menu::input::mi_update();

        // Per-frame feature loop (godmode etc. re-applied every frame, whether or
        // not the owning submenu is open), then the control manager's request
        // queues (model/asset streaming for spawns).
        TICK_TRACE("feature_update");
        menu::submenu::handler::feature_update();
        TICK_TRACE("control");
        menu::control::update();

        // Notifications + stacked display + side panels render every frame
        // (panels::update no-ops while the menu is closed).
        TICK_TRACE("notify");
        menu::notify::update();
        TICK_TRACE("display");
        menu::display::render();
        TICK_TRACE("panels");
        menu::panels::update();
        TICK_TRACE("done");

#undef TICK_TRACE
    }
}
