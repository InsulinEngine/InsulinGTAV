#include "menu/base/submenus/helper_esp.h"
#include "menu/base/submenus/helper_esp_settings.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/util/rainbow.h"

namespace {
    menu::esp::esp_context* g_current = nullptr;
    const char*             g_title   = "ESP";

    // Rebuilt whenever the target changes: the options bind to the current
    // context's fields by reference, so a stale build would edit the previous
    // consumer's context.
    menu::esp::esp_context* g_built_for = nullptr;
}

void helper_esp_menu::open_for(menu::esp::esp_context* ctx, const char* title) {
    g_current = ctx;
    g_title   = title ? title : "ESP";
}

menu::esp::esp_context* helper_esp_menu::current() { return g_current; }

void helper_esp_menu::load() {
    set_name("ESP");
    update_once();
}

void helper_esp_menu::update_once() {
    clear_options(0);
    g_built_for = g_current;

    if (!g_current) {
        add_option(button_option("~m~No ESP target").ref());
        return;
    }
    set_name(g_title);

    menu::esp::esp_context& c = *g_current;

    // Deliberately not persisted. add_savable's save key is
    // (get_submenu_name_stack(), option name) - the menu path, not the
    // consumer. helper_esp_menu is one shared editor reused for Session ESP,
    // every per-player ESP, and Vehicle ESP (see open_for() call sites), so
    // that key carries no context identity: it cannot tell "Snapline" for
    // the session apart from "Snapline" for player 7 or for vehicles. It
    // would also currently no-op regardless - this menu has no
    // set_parent<T>(), so its name stack is empty and add_savable's own
    // size<=0 guard skips it. A real fix means serialising each esp_context
    // under its own consumer identity, not calling add_savable here - do not
    // reintroduce it without that. See docs/superpowers/specs/2026-08-16-esp-design.md.
    add_option(toggle_option("Name").add_toggle(c.m_name));
    add_option(number_option<int>(SCROLL, "Name Shows")
        .add_number(c.m_name_type, "%i", 1).add_min(0).add_max(1)
        .add_tooltip("0 = name only, 1 = name and distance"));
    add_option(toggle_option("Snapline").add_toggle(c.m_snapline));
    add_option(toggle_option("2D Box").add_toggle(c.m_2d_box));
    add_option(toggle_option("2D Corners").add_toggle(c.m_2d_corners));
    add_option(toggle_option("Health Bar").add_toggle(c.m_healthbar));
    add_option(toggle_option("3D Box").add_toggle(c.m_3d_box));
    add_option(toggle_option("3D Axis").add_toggle(c.m_3d_axis));

    add_option(break_option("Peds only").ref());
    add_option(toggle_option("Skeleton - Bones").add_toggle(c.m_skeleton_bones));
    add_option(toggle_option("Skeleton - Joints").add_toggle(c.m_skeleton_joints));
    add_option(toggle_option("Weapon").add_toggle(c.m_weapon));

    add_option(break_option("Range").ref());
    // number_option, not scroll_option: scroll_option binds a scroll_struct
    // list, while these are plain bounded integers. This is the same form
    // helper_color.cpp uses for its channels, so it is known to work.
    add_option(number_option<int>(SCROLL, "Max Distance")
        .add_number(c.m_max_distance, "%im", 25).add_min(25).add_max(1000)
        .add_tooltip("Entities further away than this are not drawn at all"));
    add_option(number_option<int>(SCROLL, "Skeleton Distance")
        .add_number(c.m_skeleton_distance, "%im", 5).add_min(10).add_max(200)
        .add_tooltip("Bones are the most expensive element - keep this short"));

    add_option(submenu_option("Colours").add_submenu<helper_esp_settings_menu>());

    // Nothing above is persisted (see the comment before the toggles), so no
    // restore ever sets a rainbow flag without going through the click
    // handler - this block guards a case that cannot happen yet. It is kept
    // anyway: it costs nothing, it is the correct reconciliation for the day
    // real per-context persistence lands (add_savable restores a value but
    // deliberately does not run the click handler - a handler firing during
    // build() is what took the game down historically - so a rainbow
    // restored as `true` would have a flag set and no registration), and
    // rainbow::add() is idempotent (contains() guard - see rainbow.cpp), so
    // calling it unconditionally on every rebuild is safe.
    if (c.m_name_text_rainbow)       menu::get_rainbow()->add(&c.m_name_text_color);
    if (c.m_name_bg_rainbow)         menu::get_rainbow()->add(&c.m_name_bg_color);
    if (c.m_snapline_rainbow)        menu::get_rainbow()->add(&c.m_snapline_color);
    if (c.m_2d_box_rainbow)          menu::get_rainbow()->add(&c.m_2d_box_color);
    if (c.m_2d_corners_rainbow)      menu::get_rainbow()->add(&c.m_2d_corners_color);
    if (c.m_healthbar_rainbow)       menu::get_rainbow()->add(&c.m_healthbar_color);
    if (c.m_3d_box_rainbow)          menu::get_rainbow()->add(&c.m_3d_box_color);
    if (c.m_skeleton_bones_rainbow)  menu::get_rainbow()->add(&c.m_skeleton_bones_color);
    if (c.m_skeleton_joints_rainbow) menu::get_rainbow()->add(&c.m_skeleton_joints_color);
    if (c.m_weapon_rainbow)          menu::get_rainbow()->add(&c.m_weapon_color);
}

void helper_esp_menu::update() {
    if (g_built_for != g_current) update_once();
}

helper_esp_menu* helper_esp_menu::get() {
    static helper_esp_menu instance;
    return &instance;
}
