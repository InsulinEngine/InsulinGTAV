#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/submenu.h"
#include "menu/base/base.h"

class submenu_option : public base_option {
public:
    submenu_option(stl::string name)
        : base_option(name) {}

    template<typename T>
    submenu_option& add_submenu() { m_submenu = T::get(); return *this; }

    submenu_option& add_click(stl::function<void()> function) { m_on_click = function; return *this; }
    submenu_option& add_click_this(stl::function<void(submenu_option*)> function) { m_on_click_this = function; return *this; }
    submenu_option& add_requirement(stl::function<bool()> function) { m_requirement = function; return *this; }
    submenu_option& add_update(stl::function<void(submenu_option*, int)> function) { m_on_update = function; return *this; }
    submenu_option& add_hover(stl::function<void(submenu_option*)> function) { m_on_hover = function; return *this; }
    submenu_option& add_tooltip(stl::string tooltip) { m_tooltip.set(tooltip.c_str()); return *this; }
    submenu_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    submenu_option& add_hotkey() { m_has_hotkey = true; return *this; }
    submenu_option& add_side_text(stl::string text) { m_side_text = { true, text }; return *this; }
    submenu_option& add_offset(float offset) { m_offset = offset; return *this; }
    submenu_option& add_icon_override(stl::pair<stl::string, stl::string> asset, math::vector2<float> offset, math::vector2<float> scale, color_rgba* color) { m_override = { true, asset, offset, scale, color }; return *this; }
    submenu_option& add_custom_data(uint8_t* data) { memcpy(m_custom_data, data, 0x10); return *this; }
    submenu_option& disable_icon() { m_disable_icon = true; return *this; }

    uint8_t* get_custom_data() { return m_custom_data; }

    void render(int position);
    void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    void invoke_hotkey();
private:
    struct side_text {
        bool m_enabled = false;
        stl::string m_text = "";
    };

    struct icon_override {
        bool m_enabled = false;
        stl::pair<stl::string, stl::string> m_asset = {};
        math::vector2<float> m_offset;
        math::vector2<float> m_scale;
        color_rgba* m_color;
    };

    side_text m_side_text;
    icon_override m_override;
    float m_offset = 0.f;
    uint8_t m_custom_data[0x10];
    bool m_disable_icon = false;

    menu::submenu::submenu* m_submenu = nullptr;

    stl::function<void()> m_on_click = []() {};
    stl::function<void(submenu_option*)> m_on_click_this = [](submenu_option*) {};
    stl::function<void(submenu_option*)> m_on_hover = [](submenu_option*) {};
    stl::function<void(submenu_option*, int)> m_on_update = [](submenu_option*, int) {};
};
