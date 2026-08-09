#pragma once
#include "option.h"
#include "menu/base/base.h"
#include "rage/types/base_types.h"
#include "util/math.h"
#include "util/config.h"

class color_option : public base_option {
public:
    color_option(stl::string name)
        : base_option(name) {}

    color_option& add_color(color_rgba& color) { m_color = &color; m_color_cache = color; return *this; }
    color_option& add_click(stl::function<void()> function) { m_on_click = function; return *this; }
    color_option& add_click_this(stl::function<void(color_option*)> function) { m_on_click_this = function; return *this; }
    color_option& add_requirement(stl::function<bool()> function) { m_requirement = function; return *this; }
    color_option& add_update(stl::function<void(color_option*)> function) { m_on_update = function; return *this; }
    color_option& add_update_this(stl::function<void(color_option*, int)> function) { m_on_update_this = function; return *this; }
    color_option& add_hover(stl::function<void()> function) { m_on_hover = function; return *this; }
    color_option& add_tooltip(stl::string tooltip) { m_tooltip.set(tooltip.c_str()); return *this; }
    color_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    color_option& add_instructional(stl::string text, eScaleformButtons color_option) { m_instructionals.push_back({ text, (int)color_option, false }); return *this; }
    color_option& add_instructional(stl::string text, eControls color_option) { m_instructionals.push_back({ text, (int)color_option, true }); return *this; }
    color_option& add_savable(stl::stack<stl::string> menu_stack) {
        if (menu_stack.size() <= 0) return *this;

        m_savable = true;
        if (m_color && m_requirement()) {
            util::config::read_color(menu_stack, m_name.get_original().c_str(), m_color, { "Color" });
            m_color_cache = *m_color;
        }
        return *this;
    }

    color_rgba* get_color() { return m_color; }

    void render(int position);
    void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    void invoke_hotkey() {}
private:
    color_rgba* m_color = nullptr;
    color_rgba m_color_cache;

    stl::function<void()> m_on_click = []() {};
    stl::function<void(color_option*)> m_on_click_this = [](color_option*) {};
    stl::function<void()> m_on_hover = []() {};
    stl::function<void(color_option*)> m_on_update = [](color_option*) {};
    stl::function<void(color_option*, int)> m_on_update_this = [](color_option*, int) {};
};
