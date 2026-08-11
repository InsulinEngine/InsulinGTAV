#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/base.h"
#include "menu/base/renderer.h"
#include "menu/base/util/input.h"
#include "menu/base/util/overlay.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

// Modal option: a button that opens a centered confirm/cancel dialog. Cross opens
// it; inside, D-pad left/right picks Confirm/Cancel, Cross fires the chosen
// callback, Circle cancels.
class modal_option : public base_option {
public:
    modal_option(stl::string name) : base_option(name) {}

    modal_option& add_message(stl::string m) { m_message = m; return *this; }
    modal_option& add_confirm(stl::function<void()> f) { m_on_confirm = f; return *this; }
    modal_option& add_cancel(stl::function<void()> f) { m_on_cancel = f; return *this; }
    modal_option& add_hover(stl::function<void()> f) { m_on_hover = f; return *this; }
    modal_option& add_requirement(stl::function<bool()> f) { m_requirement = f; return *this; }
    modal_option& add_tooltip(stl::string t) { m_tooltip.set(t); return *this; }
    modal_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    modal_option& add_hotkey() { m_has_hotkey = true; return *this; }

    void render(int position) {
        bool selected = menu::base::is_option_selected(position);
        color_rgba color = selected ? global::ui::g_option_selected : global::ui::g_option;

        const float scale = menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height);
        const float row_y = global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f;

        menu::renderer::draw_text(m_name.get(), { global::ui::g_position.x + 0.004f, row_y }, scale, global::ui::g_option_font, color);
        menu::renderer::draw_text("~m~...", { global::ui::g_position.x + 0.004f, row_y }, scale, global::ui::g_option_font, color, JUSTIFY_RIGHT,
            { 0.f, (1.0f - (1.0f - (global::ui::g_position.x + (0.315f / 2.f)) - global::ui::g_wrap + (0.22f - global::ui::g_scale.x))) - 0.005f });
    }

    void render_selected(int /*position*/, stl::stack<stl::string> /*submenu_name_stack*/) {
        if (menu::overlay::active()) return;   // overlay owns input while open
        m_on_hover();

        if (m_requirement() && menu::input::is_option_pressed()) {
            menu::overlay::open_modal(m_name.get(), m_message, m_on_confirm, m_on_cancel);
        }
    }

    void invoke_hotkey() {
        if (m_requirement()) menu::overlay::open_modal(m_name.get(), m_message, m_on_confirm, m_on_cancel);
    }
private:
    stl::string m_message = "";
    stl::function<void()> m_on_confirm = []() {};
    stl::function<void()> m_on_cancel = []() {};
    stl::function<void()> m_on_hover = []() {};
};
