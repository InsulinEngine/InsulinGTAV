#pragma once
#include "platform/stdafx.h"
#include "util/localization.h"
#include "global/ui_vars.h"

// Empty defaults for the two hotkey-notify labels the option subclasses use.
// (Zero-initialised localization = empty string, which is fine here.)
static localization t_used_hotkey_for("Used hotkey for", true, true);
static localization t_hotkey("Hotkey", true, true);

class base_option {
public:
    base_option();
    base_option(stl::string name, bool breaker = false, bool slider = false, bool input = false);

    virtual void render(int position);
    virtual void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    virtual void invoke_save(stl::stack<stl::string> submenu_name_stack);
    virtual void invoke_hotkey();

    bool is_visible() { return m_visible && m_requirement(); }
    bool is_break() { return m_break_option; }
    bool is_slider() { return m_slider; }
    bool is_input() { return m_input; }
    bool is_savable() { return m_savable; }
    bool has_hotkey() { return m_has_hotkey; }
    int get_hotkey() { return m_hotkey; }

    localization& get_name() { return m_name; }
    localization& get_tooltip() { return m_tooltip; }
    stl::stack<stl::string>* get_submenu_name_stack() { return m_submenu_name_stack; }

    void set_hotkey(int key) { m_hotkey = key; }
    void set_name(stl::string name) { m_name.set(name); }
    void set_submenu_name_stack(stl::stack<stl::string>* stack) { m_submenu_name_stack = stack; }
    void set_tooltip(stl::string tooltip) { m_tooltip.set(tooltip); }
protected:
    localization m_name;
    localization m_tooltip;

    bool m_break_option = false;
    bool m_visible = true;
    bool m_savable = false;
    bool m_slider = false;
    bool m_input = false;
    bool m_has_hotkey = false;
    int m_hotkey = -1;

    stl::function<bool()> m_requirement = []() { return true; };
    stl::vector<stl::tuple3<stl::string, int, bool>> m_instructionals = {};
    stl::stack<stl::string>* m_submenu_name_stack = nullptr;
};
