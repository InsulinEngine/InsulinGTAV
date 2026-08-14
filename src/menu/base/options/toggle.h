#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/base.h"
#include "util/config.h"

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
    toggle_option& add_savable(stl::stack<stl::string> menu_stack) {
        if (menu_stack.size() <= 0) return *this;

        m_savable = true;
        if (m_toggle && m_requirement()) {
            *m_toggle = util::config::read_bool(menu_stack, m_name.get_original().c_str(), *m_toggle);
            // Deliberately NOT calling m_on_click() here. add_savable runs inside
            // menu::build(), which happens right after the frame hook is installed and
            // possibly while the game is still on its loading screen - a click handler
            // that touches natives (get_player_ped and friends) then dereferences a
            // player that does not exist yet and takes the game down. Whether it
            // crashed depended on how far the game had booted, which made it look
            // random. Savable toggles must apply their effect from feature_update,
            // which runs every frame and already re-applies them.
        }
        return *this;
    }
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
