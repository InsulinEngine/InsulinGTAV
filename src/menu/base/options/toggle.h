#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/base.h"

class toggle_option : public base_option {
public:
    toggle_option(stl::string name)
        : base_option(name)
    {}

    toggle_option& add_click(stl::function<void()> function) { m_on_click = function; return *this; }
    toggle_option& add_requirement(stl::function<bool()> function) { m_requirement = function; return *this; }
    toggle_option& add_update(stl::function<void(toggle_option*, int)> function) { m_on_update = function; return *this; }
    toggle_option& add_hover(stl::function<void()> function) { m_on_hover = function; return *this; }
    toggle_option& add_tooltip(stl::string tooltip) { m_tooltip.set(tooltip.c_str()); return *this; }
    toggle_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    toggle_option& add_hotkey() { m_has_hotkey = true; return *this; }
    toggle_option& add_toggle(bool& tog) { m_toggle = &tog; return *this; }
    // Config persistence is out of scope; kept as a no-op for call compatibility.
    toggle_option& add_savable(stl::stack<stl::string> /*menu_stack*/) { return *this; }
    toggle_option& add_offset(float offset) { m_offset = offset; return *this; }

    void render(int position);
    void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    void invoke_hotkey();
private:
    float m_offset = 0.f;
    bool* m_toggle = nullptr;

    stl::function<void()> m_on_click = []() {};
    stl::function<void()> m_on_hover = []() {};
    stl::function<void(toggle_option*, int)> m_on_update = [](toggle_option*, int) {};
};
