#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/base.h"
#include "menu/base/renderer.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "util/config.h"
#include "stl/type_traits.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"   // play_sound_frontend
#include <stdio.h>

// Scrollbar / slider option: a numeric value drawn as a filled progress bar on
// the right of the row (fraction = (value-min)/(max-min)), with the value text.
// D-pad left/right adjusts by step (with hold-to-repeat), like number_option.
// Header-only template (mirrors scroll_option) so no explicit instantiation.
template<typename Type>
class scrollbar_option : public base_option {
public:
    scrollbar_option(stl::string name)
        : base_option(name, false, /*slider*/true) {}

    scrollbar_option& add_number(Type& num, stl::string fmt, Type step) { m_number = &num; m_number_cache = num; m_format = fmt; m_step = step; return *this; }
    scrollbar_option& add_min(Type m) { m_min = m; return *this; }
    scrollbar_option& add_max(Type m) { m_max = m; return *this; }
    scrollbar_option& add_click(stl::function<void()> f) { m_on_click = f; return *this; }
    scrollbar_option& add_hover(stl::function<void()> f) { m_on_hover = f; return *this; }
    scrollbar_option& add_update(stl::function<void(scrollbar_option*, int)> f) { m_on_update = f; return *this; }
    scrollbar_option& add_requirement(stl::function<bool()> f) { m_requirement = f; return *this; }
    scrollbar_option& add_tooltip(stl::string t) { m_tooltip.set(t); return *this; }
    scrollbar_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    scrollbar_option& add_hotkey() { m_has_hotkey = true; return *this; }
    scrollbar_option& set_scroll_speed(uint32_t s) { m_scroll_speed = s; return *this; }
    scrollbar_option& hide_value() { m_show_value = false; return *this; }
    scrollbar_option& add_offset(float o) { m_offset = o; return *this; }
    scrollbar_option& add_savable(stl::stack<stl::string> menu_stack) {
        if (menu_stack.size() <= 0) return *this;
        m_savable = true;
        if (m_number && m_requirement()) {
            if (stl::is_same<Type, float>::value)
                *m_number = (Type)util::config::read_float(menu_stack, m_name.get_original().c_str(), (float)*m_number, { "Values" });
            else
                *m_number = (Type)util::config::read_int(menu_stack, m_name.get_original().c_str(), (int)*m_number, { "Values" });
            clamp();
            m_number_cache = *m_number;
        }
        return *this;
    }

    void render(int position) {
        bool selected = menu::base::is_option_selected(position);
        color_rgba color = selected ? global::ui::g_option_selected : global::ui::g_option;
        m_on_update(this, position);

        const float row_y = global::ui::g_position.y + (position * global::ui::g_option_scale);

        // label (left)
        menu::renderer::draw_text(m_name.get(),
            { global::ui::g_position.x + 0.004f + m_offset, row_y + 0.004f },
            menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height),
            global::ui::g_option_font, color);

        // fraction filled
        float frac = 0.f;
        if (m_number && m_max > m_min) frac = (float)((double)(*m_number - m_min) / (double)(m_max - m_min));
        if (frac < 0.f) frac = 0.f;
        if (frac > 1.f) frac = 1.f;

        // bar geometry: right-aligned inside the menu, vertically centred in row
        const float bar_w = 0.075f;
        const float bar_h = 0.009f;
        const float right_edge = global::ui::g_position.x + global::ui::g_scale.x - 0.008f;
        const float bar_x = right_edge - bar_w;
        const float bar_y = row_y + (global::ui::g_option_scale * 0.5f) - (bar_h * 0.5f);

        // framed track (border for contrast) with the fill on top
        menu::renderer::draw_outlined_rect({ bar_x, bar_y }, { bar_w, bar_h }, 0.0012f, global::ui::g_toggle_off.opacity(90), global::ui::g_option);
        if (frac > 0.f) menu::renderer::draw_rect({ bar_x, bar_y }, { bar_w * frac, bar_h }, color_rgba(40, 40, 40, 255));

        // value text, right-justified just left of the bar
        if (m_show_value && m_number) {
            char buf[64];
            snprintf(buf, sizeof(buf), m_format.c_str(), *m_number);
            menu::renderer::draw_text(buf, { bar_x - 0.006f, row_y + 0.004f },
                menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height) * 0.88f,
                global::ui::g_option_font, color, JUSTIFY_RIGHT, { 0.f, bar_x - 0.006f });
        }
    }

    void render_selected(int position, stl::stack<stl::string> submenu_name_stack) {
        m_on_hover();

        if (m_requirement() && menu::input::is_option_pressed()) m_on_click();

        static uint32_t timer = 0;

        if (menu::input::is_left_just_pressed()) {
            if (!m_left_disabled) step(-1);
            m_left_disabled = false; m_left_timer = 0;
        }
        if (menu::input::is_right_just_pressed()) {
            if (!m_right_disabled) step(+1);
            m_right_disabled = false; m_right_timer = 0;
        }
        if (menu::input::is_left_pressed()) {
            if (++m_left_timer > 20) { m_left_disabled = true; if ((platform::now_ms() - timer) > m_scroll_speed) { step(-1); timer = platform::now_ms(); } }
        }
        if (menu::input::is_right_pressed()) {
            if (++m_right_timer > 20) { m_right_disabled = true; if ((platform::now_ms() - timer) > m_scroll_speed) { step(+1); timer = platform::now_ms(); } }
        }

        // persist on change
        if (m_savable && m_number && m_number_cache != *m_number) {
            m_number_cache = *m_number;
            if (stl::is_same<Type, float>::value) util::config::write_float(submenu_name_stack, m_name.get_original(), (float)*m_number, { "Values" });
            else util::config::write_int(submenu_name_stack, m_name.get_original(), (int)*m_number, { "Values" });
        }
    }

    void invoke_hotkey() { if (m_requirement()) m_on_click(); }
private:
    void clamp() { if (!m_number) return; if (*m_number < m_min) *m_number = m_min; if (*m_number > m_max) *m_number = m_max; }
    void step(int dir) {
        if (!m_number) return;
        *m_number += (Type)(dir > 0 ? m_step : -m_step);
        clamp();
        m_on_click();
        native::play_sound_frontend(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
    }

    stl::function<void()> m_on_click = []() {};
    stl::function<void()> m_on_hover = []() {};
    stl::function<void(scrollbar_option*, int)> m_on_update = [](scrollbar_option*, int) {};

    Type* m_number = nullptr;
    Type m_step = (Type)1;
    Type m_min = (Type)0;
    Type m_max = (Type)0;
    stl::string m_format = "%d";
    bool m_show_value = true;
    float m_offset = 0.f;
    uint32_t m_scroll_speed = 60;

    int m_left_timer = 0;
    int m_right_timer = 0;
    bool m_left_disabled = false;
    bool m_right_disabled = false;
    Type m_number_cache = (Type)0;
};
