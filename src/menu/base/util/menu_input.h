#pragma once
#include "platform/stdafx.h"
#include "menu/base/options/option.h"
#include "global/ui_vars.h"

// menu_input: the deferred-action queue + a minimal HSV colour modal. The full
// grid colour picker (Ozark phase 2b) is deferred; this is a working D-pad/
// shoulder HSV adjuster. Hotkey capture stays stubbed (feature).
namespace menu::input {
    class menu_input {
    public:
        void update();
        void push(stl::function<void()> function);
        void hotkey(stl::string name, base_option* option);
        void color(color_rgba* option);
        int get_key(stl::string name, int default_key);

        bool is_color_active() { return m_color_active; }
    private:
        stl::vector<stl::function<void()>> m_queue;

        bool m_color_active = false;
        color_rgba* m_color_target = nullptr;
        color_hsv m_hsv = { 0.f, 0.f, 0.f };
        int m_alpha = 255;
    };

    menu_input* get_menu_input();

    inline void mi_update() { get_menu_input()->update(); }
    inline void push(stl::function<void()> function) { get_menu_input()->push(function); }
    inline void hotkey(stl::string name, base_option* option) { get_menu_input()->hotkey(name, option); }
    inline void color(color_rgba* color) { get_menu_input()->color(color); }
    inline int get_key(stl::string name, int default_key) { return get_menu_input()->get_key(name, default_key); }
    inline bool is_color_active() { return get_menu_input()->is_color_active(); }

    // Key-name table (index 0 used on PS4; open bind is L1+O, keyboard key unused).
    static const char* g_key_names[254] = { "" };
}
