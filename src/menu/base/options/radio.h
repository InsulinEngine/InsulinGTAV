#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/base.h"

class radio_option : public base_option {
public:
    radio_option(stl::string name)
        : base_option(name) {}

    radio_option& add_radio(radio_context& radio) {
        m_context = &radio;
        m_index = m_context->m_count;
        m_id = m_context->m_count;
        m_context->m_count++;

        if (m_id == 0) {
            m_context->m_toggles[m_index] = true;
        }

        return *this;
    }

    radio_option& add_click(stl::function<void()> function) { m_on_click = function; return *this; }
    radio_option& add_requirement(stl::function<bool()> function) { m_requirement = function; return *this; }
    radio_option& add_update(stl::function<void(radio_option*)> function) { m_on_update = function; return *this; }
    radio_option& add_hover(stl::function<void()> function) { m_on_hover = function; return *this; }
    radio_option& add_tooltip(stl::string tooltip) { m_tooltip.set(tooltip.c_str()); return *this; }
    radio_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }

    void update_selected() {
        m_context->m_toggles[m_index] = true;
        m_context->m_toggles[m_context->m_selected] = false;
        m_context->m_selected = m_id;
    }

    bool is_selected() { return m_context->m_selected == m_id; }

    void render(int position);
    void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    void invoke_hotkey() {}
private:
    stl::function<void()> m_on_click = []() {};
    stl::function<void()> m_on_hover = []() {};
    stl::function<void(radio_option*)> m_on_update = [](radio_option*) {};

    int m_id = 0;
    int m_index = 0;
    radio_context* m_context = nullptr;
};
