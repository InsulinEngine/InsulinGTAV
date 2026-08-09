#include "submenu_option.h"
#include "menu/base/renderer.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "menu/base/submenu_handler.h"
#include "rage/invoker/natives.h"

void submenu_option::render(int position) {
    bool selected = menu::base::is_option_selected(position);
    color_rgba color = selected ? global::ui::g_option_selected : global::ui::g_option;
    color_rgba bar_color = global::ui::g_submenu_bar;

    m_on_update(this, position);

    menu::renderer::draw_text(m_name.get(), { global::ui::g_position.x + 0.004f + m_offset, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, color);

    if (!m_disable_icon) {
        if (m_override.m_enabled) {
            menu::renderer::draw_sprite_aligned(m_override.m_asset, { global::ui::g_position.x + m_override.m_offset.x - (0.23f - global::ui::g_scale.x), global::ui::g_position.y + (position * global::ui::g_option_scale) + m_override.m_offset.y }, m_override.m_scale, 0.f, *m_override.m_color);
        } else {
            // Ozark draws a custom arrow.png here; without the asset pipeline that
            // renders as a placeholder square (same as the toggle). Draw a ">"
            // chevron glyph instead: asset-free, right-aligned, and clearly a
            // submenu indicator distinct from the toggle.
            menu::renderer::draw_text(">", { global::ui::g_position.x + 0.004f, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, bar_color, JUSTIFY_RIGHT, { 0.f, (1.0f - (1.0f - (global::ui::g_position.x + (0.315f / 2.f)) - global::ui::g_wrap)) - 0.005f });
        }
    }

    if (m_side_text.m_enabled) {
        menu::renderer::draw_text(m_side_text.m_text, { global::ui::g_position.x + 0.004f, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, color, JUSTIFY_RIGHT, { 0.f, (1.0f - (1.0f - (global::ui::g_position.x + (0.315f / 2.f)) - global::ui::g_wrap)) - 0.005f });
    }
}

void submenu_option::render_selected(int position, stl::stack<stl::string> submenu_name_stack) {
    m_on_hover(this);

    if (m_requirement() && menu::input::is_option_pressed()) {
        m_on_click();
        m_on_click_this(this);
        menu::submenu::handler::set_submenu(m_submenu);
    }
}

// Hotkey activation (feature): open + navigate to the submenu; no notify/va.
void submenu_option::invoke_hotkey() {
    if (!m_requirement()) return;
    m_on_click();

    if (!menu::base::is_open()) menu::base::set_open(true);
    menu::submenu::handler::set_submenu(m_submenu);
}
