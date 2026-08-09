#include "toggle.h"
#include "menu/base/renderer.h"
#include "menu/base/util/instructionals.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "menu/base/util/hotkeys.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

void toggle_option::render(int position) {
    bool selected = menu::base::is_option_selected(position);
    color_rgba color = selected ? global::ui::g_option_selected : global::ui::g_option;
    color_rgba toggle_color = (m_toggle && *m_toggle) ? global::ui::g_toggle_on : global::ui::g_toggle_off;

    m_on_update(this, position);

    menu::renderer::draw_text(m_name.get(), { global::ui::g_position.x + 0.004f + m_offset, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, color);
    // Ozark uses a custom toggle_circle.png; without assets draw a solid colour
    // quad via the sentinel dict (green when on, red when off).
    menu::renderer::draw_sprite({ "randomha", "" }, { global::ui::g_position.x + global::ui::g_toggle_position.x - (0.23f - global::ui::g_scale.x), global::ui::g_position.y + global::ui::g_toggle_position.y + (position * global::ui::g_option_scale) }, global::ui::g_toggle_scale, 0.f, toggle_color);
}

void toggle_option::render_selected(int position, stl::stack<stl::string> submenu_name_stack) {
    m_on_hover();

    if (m_toggle && m_requirement() && menu::input::is_option_pressed()) {
        *m_toggle = !*m_toggle;
        m_on_click();
    }
}

// Hotkey activation (feature): flip + click only; no notify/va path.
void toggle_option::invoke_hotkey() {
    if (!m_requirement()) return;
    if (m_toggle) *m_toggle = !*m_toggle;
    m_on_click();
}
