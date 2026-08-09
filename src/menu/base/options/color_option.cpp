#include "color_option.h"
#include "menu/base/renderer.h"
#include "menu/base/util/instructionals.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"

void color_option::render(int position) {
    bool selected = menu::base::is_option_selected(position);
    color_rgba color = selected ? global::ui::g_option_selected : global::ui::g_option;

    m_on_update(this);
    m_on_update_this(this, position);

    menu::renderer::draw_text(m_name.get(), { global::ui::g_position.x + 0.004f, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, color);
    menu::renderer::draw_rect({ global::ui::g_position.x + 0.2145f - (0.23f - global::ui::g_scale.x), global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.0055f }, { 0.01f, 0.02f }, *m_color);
}

void color_option::render_selected(int position, stl::stack<stl::string> submenu_name_stack) {
    m_on_hover();

    if (m_instructionals.size()) {
        instructionals::setup();

        for (stl::tuple3<stl::string, int, bool>& instructional : m_instructionals) {
            if (instructional.c) {
                instructionals::add_instructional(instructional.a, (eControls)instructional.b);
            } else {
                instructionals::add_instructional(instructional.a, (eScaleformButtons)instructional.b);
            }
        }

        instructionals::close();
    }

    if (m_requirement() && menu::input::is_option_pressed()) {
        m_on_click();
        m_on_click_this(this);

        menu::input::push([this] {
            menu::input::color(m_color);
        });
    }
}
