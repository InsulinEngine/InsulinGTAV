#pragma once
#include "option.h"
#include "menu/base/base.h"
#include "rage/types/base_types.h"
#include "util/math.h"

class button_option : public base_option {
public:
    button_option(stl::string name)
        : base_option(name) {}

    button_option& ref() { return *this; }

    button_option& add_click(stl::function<void()> function) { m_on_click = function; return *this; }
    button_option& add_click_this(stl::function<void(button_option*)> function) { m_on_click_this = function; return *this; }
    button_option& add_requirement(stl::function<bool()> function) { m_requirement = function; return *this; }
    button_option& add_update(stl::function<void(button_option*)> function) { m_on_update = function; return *this; }
    button_option& add_update_this(stl::function<void(button_option*, int)> function) { m_on_update_this = function; return *this; }
    button_option& add_hover(stl::function<void()> function) { m_on_hover = function; return *this; }
    button_option& add_tooltip(stl::string tooltip) { m_tooltip.set(tooltip.c_str()); return *this; }
    button_option& add_keyboard(stl::string title, int max_chars, stl::function<void(button_option*, const char*)> function) { m_keyboard = { true, false, max_chars, function, title }; return *this; }
    button_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    button_option& remove_sprite() { m_sprite.m_enabled = false; return *this; }
    button_option& add_sprite(stl::pair<stl::string, stl::string> asset, stl::function<bool()> requirement = [] { return true; }) { m_sprite = { true, asset, requirement }; return *this; }
    button_option& add_sprite_scale(math::vector2<float> scale) { m_sprite.m_scale = scale; return *this; }
    button_option& add_sprite_rotation() { m_sprite.m_rotate = true; return *this; }
    button_option& add_instructional(stl::string text, eScaleformButtons button_option) { m_instructionals.push_back({ text, (int)button_option, false }); return *this; }
    button_option& add_instructional(stl::string text, eControls button_option) { m_instructionals.push_back({ text, (int)button_option, true }); return *this; }
    button_option& add_side_text(stl::string text) { m_side_text = { true, text }; return *this; }
    button_option& add_hotkey() { m_has_hotkey = true; return *this; }
    button_option& add_offset(float offset) { m_offset = offset; return *this; }
    button_option& add_keyboard_default(stl::string de) { m_keyboard.m_default_text = de; return *this; }

    void render(int position);
    void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    void invoke_hotkey();
private:
    struct Keyboard {
        bool m_enabled = false;
        bool m_is_active = false;
        int m_max_chars = 0;
        stl::function<void(button_option*, const char*)> m_callback = {};
        stl::string m_title = "";
        stl::string m_default_text = "";
    };

    struct Sprite {
        bool m_enabled = false;
        stl::pair<stl::string, stl::string> m_asset = {};
        stl::function<bool()> m_requirement = {};
        math::vector2<float> m_scale = { 0.f, 0.f };
        bool m_rotate = false;
        float m_rotation = 0.f;
    };

    struct SideText {
        bool m_enabled = false;
        stl::string m_text = "";
    };

    Keyboard m_keyboard;
    Sprite m_sprite;
    SideText m_side_text;
    float m_offset = 0.f;

    stl::function<void()> m_on_click = []() {};
    stl::function<void(button_option*)> m_on_click_this = [](button_option*) {};
    stl::function<void()> m_on_hover = []() {};
    stl::function<void(button_option*)> m_on_update = [](button_option*) {};
    stl::function<void(button_option*, int)> m_on_update_this = [](button_option*, int) {};
};
