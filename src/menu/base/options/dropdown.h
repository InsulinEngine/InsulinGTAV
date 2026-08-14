#pragma once
#include "platform/stdafx.h"
#include "option.h"
#include "menu/base/base.h"
#include "menu/base/renderer.h"
#include "menu/base/util/input.h"
#include "menu/base/util/overlay.h"
#include "util/config.h"
#include "stl/vector.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"   // play_sound_frontend
#include <stdio.h>

// Dropdown option: shows the current choice inline; Cross opens a centered list
// overlay to pick from `m_items`; D-pad left/right also cycles inline. Bound to an
// int index. Header-only template-free class.
class dropdown_option : public base_option {
public:
    dropdown_option(stl::string name) : base_option(name, false, /*slider*/true) {}

    dropdown_option& add_index(int& idx) { m_index = &idx; m_index_cache = idx; return *this; }
    dropdown_option& add_item(stl::string s) { m_items.push_back(s); return *this; }
    dropdown_option& add_items(stl::vector<stl::string> v) { for (size_t i = 0; i < v.size(); i++) m_items.push_back(v[i]); return *this; }
    dropdown_option& add_change(stl::function<void(int)> f) { m_on_change = f; return *this; }
    dropdown_option& add_hover(stl::function<void()> f) { m_on_hover = f; return *this; }
    dropdown_option& add_requirement(stl::function<bool()> f) { m_requirement = f; return *this; }
    dropdown_option& add_tooltip(stl::string t) { m_tooltip.set(t); return *this; }
    dropdown_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }
    dropdown_option& add_hotkey() { m_has_hotkey = true; return *this; }
    dropdown_option& add_savable(stl::stack<stl::string> menu_stack) {
        if (menu_stack.size() <= 0) return *this;
        m_savable = true;
        if (m_index && m_requirement()) {
            *m_index = util::config::read_int(menu_stack, m_name.get_original().c_str(), *m_index, { "Values" });
            clamp();
            m_index_cache = *m_index;
        }
        return *this;
    }

    void render(int position) {
        bool selected = menu::base::is_option_selected(position);
        color_rgba color = selected ? global::ui::g_option_selected : global::ui::g_option;

        const float scale = menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height);
        const float row_y = global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f;

        menu::renderer::draw_text(m_name.get(), { global::ui::g_position.x + 0.004f, row_y }, scale, global::ui::g_option_font, color);

        char buf[256];
        const char* cur = (m_index && *m_index >= 0 && *m_index < (int)m_items.size()) ? m_items[*m_index].c_str() : "";
        snprintf(buf, sizeof(buf), "%s ~m~[%i/%i]", cur, (m_index ? *m_index : 0) + 1, (int)m_items.size());
        menu::renderer::draw_text(buf, { global::ui::g_position.x + 0.004f, row_y }, scale, global::ui::g_option_font, color, JUSTIFY_RIGHT,
            { 0.f, (1.0f - (1.0f - (global::ui::g_position.x + (0.315f / 2.f)) - global::ui::g_wrap + (0.22f - global::ui::g_scale.x))) - 0.005f });
    }

    void render_selected(int position, stl::stack<stl::string> submenu_name_stack) {
        if (menu::overlay::active()) return;   // overlay owns input while open
        m_on_hover();

        const int n = (int)m_items.size();
        if (!m_index || n <= 0) return;

        if (m_requirement() && menu::input::is_option_pressed()) {
            menu::overlay::open_dropdown(m_name.get(), m_items, *m_index, [this, submenu_name_stack](int chosen) {
                *m_index = chosen;
                if (m_savable) util::config::write_int(submenu_name_stack, m_name.get_original(), chosen, { "Values" });
                m_on_change(chosen);
            });
            return;
        }

        if (menu::input::is_left_just_pressed())  { *m_index = (*m_index - 1 + n) % n; changed(submenu_name_stack); }
        if (menu::input::is_right_just_pressed()) { *m_index = (*m_index + 1) % n; changed(submenu_name_stack); }
    }

    void invoke_hotkey() {}
private:
    void clamp() { if (!m_index) return; int n = (int)m_items.size(); if (n <= 0) return; if (*m_index < 0) *m_index = 0; if (*m_index >= n) *m_index = n - 1; }
    void changed(stl::stack<stl::string>& stack) {
        if (m_savable) util::config::write_int(stack, m_name.get_original(), *m_index, { "Values" });
        m_on_change(*m_index);
        native::play_sound_frontend(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
    }

    int* m_index = nullptr;
    int m_index_cache = 0;
    stl::vector<stl::string> m_items;
    stl::function<void(int)> m_on_change = [](int) {};
    stl::function<void()> m_on_hover = []() {};
};
