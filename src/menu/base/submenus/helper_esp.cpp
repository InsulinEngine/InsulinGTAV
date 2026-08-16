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

void helper_esp_menu::set_target(menu::esp::esp_context* ctx, const char* title) {
    g_current = ctx;
    g_title   = title ? title : "ESP";
}

menu::esp::esp_context* helper_esp_menu::current() { return g_current; }

void helper_esp_menu::load() {
    set_name("ESP");
    // No set_parent<T>() here on purpose: this menu has several openers and
    // "back" must return to the one the user actually came through, so the
    // parent is set per opening by open_for<T>() instead. A parent fixed here
    // would be wrong for every opener but one.
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
    // the session apart from "Snapline" for player 7 or for vehicles - all of
    // them would read and write the same stored value. (This used to be a
    // no-op on top of being wrong, because the menu had no parent and so an
    // empty name stack, which add_savable's own size<=0 guard rejects. It now
    // gets a parent per opening, so the guard no longer covers for it.) A real
    // fix means serialising each esp_context under its own consumer identity,
    // not calling add_savable here - do not reintroduce it without that. See
    // docs/superpowers/specs/2026-08-16-esp-design.md.
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

    // Only for a context whose entities are peds. esp.cpp gates all four of
    // these on ctx.m_ped, so on Vehicle ESP they are settings that cannot do
    // anything: the toggle moves, the skeleton never appears, and the only
    // thing the user learns is that the ESP looks broken. A break with nothing
    // under it would be just as odd, so it goes inside the guard too.
    if (c.m_ped) {
        add_option(break_option("Peds only").ref());
        add_option(toggle_option("Skeleton - Bones").add_toggle(c.m_skeleton_bones));
        add_option(toggle_option("Skeleton - Joints").add_toggle(c.m_skeleton_joints));
        add_option(toggle_option("Weapon").add_toggle(c.m_weapon));
    }

    add_option(break_option("Range").ref());
    // number_option, not scroll_option: scroll_option binds a scroll_struct
    // list, while these are plain bounded integers. This is the same form
    // helper_color.cpp uses for its channels, so it is known to work.
    add_option(number_option<int>(SCROLL, "Max Distance")
        .add_number(c.m_max_distance, "%im", 25).add_min(25).add_max(1000)
        .add_tooltip("Entities further away than this are not drawn at all"));
    if (c.m_ped) {
        add_option(number_option<int>(SCROLL, "Skeleton Distance")
            .add_number(c.m_skeleton_distance, "%im", 5).add_min(10).add_max(200)
            .add_tooltip("Bones are the most expensive element - keep this short"));
    }

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
    // add() alone is not enough: rainbow::run() early-returns while m_enabled
    // is false, and nothing else in the plugin sets it - the chrome colour
    // editor sets it from its own toggle. Registering without it is the exact
    // shape of the bug this reconciliation would otherwise reintroduce.
    menu::rainbow* rb = menu::get_rainbow();
    auto reg = [rb](bool on, color_rgba* colour) {
        if (!on) return;
        rb->add(colour);
        rb->m_enabled = true;
    };
    reg(c.m_name_text_rainbow,       &c.m_name_text_color);
    reg(c.m_name_bg_rainbow,         &c.m_name_bg_color);
    reg(c.m_snapline_rainbow,        &c.m_snapline_color);
    reg(c.m_2d_box_rainbow,          &c.m_2d_box_color);
    reg(c.m_2d_corners_rainbow,      &c.m_2d_corners_color);
    reg(c.m_healthbar_rainbow,       &c.m_healthbar_color);
    reg(c.m_3d_box_rainbow,          &c.m_3d_box_color);
    reg(c.m_skeleton_bones_rainbow,  &c.m_skeleton_bones_color);
    reg(c.m_skeleton_joints_rainbow, &c.m_skeleton_joints_color);
    reg(c.m_weapon_rainbow,          &c.m_weapon_color);
}

void helper_esp_menu::update() {
    if (g_built_for != g_current) update_once();
}

helper_esp_menu* helper_esp_menu::get() {
    static helper_esp_menu instance;
    return &instance;
}
