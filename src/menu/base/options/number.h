#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/base.h"
#include "util/config.h"
#include "stl/type_traits.h"

template<typename Type>
class number_option : public base_option {
public:
    number_option(scroll_option_type type, stl::string name)
        : base_option(name, false, type, type == SCROLL), m_type(type)
    {}

    number_option& add_toggle(bool& t) { m_toggle = &t; m_toggle_cache = t; return *this; }
    number_option& add_number(Type& num, stl::string fmt, Type step) { m_number = &num; m_number_cache = num; m_format = fmt; m_step = step; return *this; }
    number_option& add_min(Type m) { m_min = m; m_has_min = true; return *this; }
    number_option& add_max(Type m) { m_max = m; m_has_max = true; return *this; }
    number_option& add_click(stl::function<void()> func) { m_on_click = func; return *this; }
    number_option& add_hover(stl::function<void()> func) { m_on_hover = func; return *this; }
    number_option& add_update(stl::function<void(number_option*, int)> func) { m_on_update = func; return *this; }
    number_option& add_requirement(stl::function<bool()> func) { m_requirement = func; return *this; }
    number_option& show_max() { m_show_max = true; return *this; }
    number_option& add_tooltip(stl::string tip) { m_tooltip.set(tip); return *this; }
    number_option& set_scroll_speed(uint32_t speed) { m_scroll_speed = speed; return *this; }
    number_option& can_loop() { m_loop = true; return *this; }
    number_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    number_option& add_hotkey() { m_has_hotkey = true; return *this; }
    number_option& add_savable(stl::stack<stl::string> menu_stack) {
        if (menu_stack.size() <= 0) return *this;

        m_savable = true;
        if (m_number && m_requirement()) {
            if (stl::is_same<Type, float>::value) {
                *m_number = (Type)util::config::read_float(menu_stack, m_name.get_original().c_str(), (float)*m_number, { "Values" });
            } else {
                *m_number = (Type)util::config::read_int(menu_stack, m_name.get_original().c_str(), (int)*m_number, { "Values" });
            }

            if (m_type == TOGGLE && m_toggle) {
                *m_toggle = util::config::read_bool(menu_stack, m_name.get_original().c_str(), *m_toggle);
                m_toggle_cache = *m_toggle;
                // No m_on_click() here - same rule as toggle_option::add_savable.
                // add_savable runs inside menu::build(), which can be while the
                // game is still on its loading screen, and a click handler that
                // touches natives dereferences a player that does not exist yet.
                // Restoring sets the value; applying it belongs in feature_update.
            }

            if (m_has_min && *m_number < m_min) *m_number = m_min;
            if (m_has_max && *m_number > m_max) *m_number = m_max;
        }
        return *this;
    }
    number_option& add_offset(float offset) { m_offset = offset; return *this; }

    void render(int position);
    void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    void invoke_hotkey();
private:
    stl::function<void()> m_on_click = []() {};
    stl::function<void()> m_on_hover = []() {};
    stl::function<void(number_option<Type>*, int)> m_on_update = [](number_option<Type>*, int) {};

    bool* m_toggle = nullptr;
    bool m_has_min = false;
    bool m_has_max = false;
    bool m_loop = false;
    bool m_show_max = false;
    bool m_keyboard_active = false;

    scroll_option_type m_type = SCROLL;
    stl::string m_format = "";
    Type* m_number = nullptr;
    Type m_step = (Type)0;
    Type m_min = (Type)0;
    Type m_max = (Type)0;
    uint32_t m_scroll_speed = 100;

    int m_left_timer = 0;
    int m_right_timer = 0;
    bool m_left_disabled = false;
    bool m_right_disabled = false;
    float m_offset = 0.f;

    bool m_toggle_cache = false;
    Type m_number_cache = (Type)0;
};
