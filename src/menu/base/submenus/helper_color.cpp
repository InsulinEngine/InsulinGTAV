#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenus/settings_themes.h"
#include "menu/base/submenus/helper_color_presets.h"
#include "menu/base/submenus/helper_color_sync.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/toggle.h"
#include "menu/base/renderer.h"
#include "menu/base/util/theme.h"
#include "menu/base/util/rainbow.h"

namespace {
    int  g_target = 0;      // index into the colour registry
    int  g_format = 0;      // 0 = RGBA, 1 = HSVA
    int  g_built_format = -1;
    color_hsv g_hsv;
    // Bound to the Rainbow toggle_option. toggle_option only invokes its click
    // handler and renders on/off state when m_toggle is non-null (toggle.cpp),
    // so this has to exist and be kept in step with the rainbow's real
    // membership - refreshed from menu::get_rainbow()->contains() every time
    // update_once() rebuilds for a (possibly new) target.
    bool g_rainbow_on = false;

    // Filled in load(), not aggregate-initialised here: localization's
    // constructor is not constexpr, so a static initializer for "RGBA"/"HSVA"
    // would need .init_array, which GoldHEN never runs (see ui_vars.h). Every
    // other scroll_struct table in this codebase (world_weather.cpp,
    // player.cpp's merryweather list, ...) follows the same pattern.
    scroll_struct<int> g_formats[2];

    color_rgba* current() { return menu::theme::color_ptr(g_target); }

    void preview() {
        if (!current()) return;
        menu::renderer::render_color_preview(*current());
    }

    // HSVA edits go through a scratch color_hsv, so push the result back after
    // every change.
    void from_hsv() {
        if (!current()) return;
        *current() = menu::renderer::hsv_to_rgb(g_hsv.h, g_hsv.s / 100.f,
                                                g_hsv.v / 100.f, current()->a);
    }
}

void helper_color_menu::target(int registry_index) {
    if (registry_index < 0 || registry_index >= menu::theme::color_count()) return;
    g_target = registry_index;
    g_built_format = -1;   // force a rebuild: the title and bindings changed
}

int         helper_color_menu::current_target() { return g_target; }
color_rgba* helper_color_menu::target_color()   { return current(); }

void helper_color_menu::load() {
    set_name("Color");
    // Themes is the only opener today. A shared editor opened from somewhere
    // else later (e.g. a per-vehicle colour picker) needs the opener to
    // re-point the parent via set_parent<T>() before showing this submenu,
    // or "back" will return to Themes regardless of who opened it.
    set_parent<settings_themes_menu>();

    g_formats[0].m_name.set("RGBA"); g_formats[0].m_result = 0;
    g_formats[1].m_name.set("HSVA"); g_formats[1].m_result = 1;

    add_option(submenu_option("Presets").add_submenu<helper_color_presets_menu>());
    add_option(submenu_option("Sync With...").add_submenu<helper_color_sync_menu>());

    add_option(toggle_option("Rainbow")
        .add_tooltip("Cycle this colour through the hue wheel")
        .add_toggle(g_rainbow_on)
        .add_click([] {
            // toggle_option flips g_rainbow_on before calling this handler, so
            // act on its new value rather than re-deriving membership.
            color_rgba* t = current();
            menu::rainbow* rb = menu::get_rainbow();
            if (g_rainbow_on) { rb->add(t); rb->m_enabled = true; }
            else                rb->remove(t);
        }));

    add_option(scroll_option<int>(SCROLL, "Color Format")
        .add_scroll(g_format, 0, 2, g_formats)
        .add_tooltip("Edit as red/green/blue or hue/saturation/value")
        .add_hover([] { preview(); }));

    add_option(break_option("Channels").ref());
}

void helper_color_menu::update() {
    // Dirty-flag rebuild: only when the format or the target actually changed.
    // Rebuilding per frame would allocate an option set every frame.
    if (g_built_format != g_format) update_once();
}

void helper_color_menu::update_once() {
    g_built_format = g_format;
    set_name(menu::theme::color_display_name(g_target), false, false);
    // Refresh from the rainbow's actual membership before the options rebuild:
    // this runs on a target() switch too, and the toggle must reflect the new
    // target's state, not the previous one's.
    g_rainbow_on = menu::get_rainbow()->contains(current());
    clear_options(5);

    if (!current()) return;

    if (g_format == 0) {
        add_option(number_option<int>(SCROLL, "Red")
            .add_number(current()->r, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        add_option(number_option<int>(SCROLL, "Green")
            .add_number(current()->g, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        add_option(number_option<int>(SCROLL, "Blue")
            .add_number(current()->b, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        add_option(number_option<int>(SCROLL, "Alpha")
            .add_number(current()->a, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        return;
    }

    g_hsv = menu::renderer::rgb_to_hsv(*current());
    g_hsv.s *= 100.f;
    g_hsv.v *= 100.f;

    add_option(number_option<float>(SCROLL, "Hue")
        .add_number(g_hsv.h, "%.2f", 1.f).add_min(0.f).add_max(360.f).can_loop()
        .add_click([] { from_hsv(); }).add_hover([] { preview(); }));
    add_option(number_option<float>(SCROLL, "Saturation")
        .add_number(g_hsv.s, "%.2f", 1.f).add_min(0.f).add_max(100.f)
        .add_click([] { from_hsv(); }).add_hover([] { preview(); }));
    add_option(number_option<float>(SCROLL, "Value")
        .add_number(g_hsv.v, "%.2f", 1.f).add_min(0.f).add_max(100.f)
        .add_click([] { from_hsv(); }).add_hover([] { preview(); }));
    add_option(number_option<int>(SCROLL, "Alpha")
        .add_number(current()->a, "%i", 1).add_min(0).add_max(255).can_loop()
        .add_hover([] { preview(); }));
}

helper_color_menu* helper_color_menu::get() {
    static helper_color_menu instance;
    return &instance;
}
