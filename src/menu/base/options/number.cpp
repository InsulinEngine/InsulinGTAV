#include "number.h"
#include "menu/base/renderer.h"
#include "menu/base/util/instructionals.h"
#include "menu/base/util/input.h"
#include "menu/base/util/menu_input.h"
#include "menu/base/util/hotkeys.h"
#include "util/config.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include <stdlib.h>

template<typename Type>
void number_option<Type>::render(int position) {
    bool selected = menu::base::is_option_selected(position);
    color_rgba color = selected ? global::ui::g_option_selected : global::ui::g_option;

    m_on_update(this, position);

    menu::renderer::draw_text(m_name.get(), { global::ui::g_position.x + 0.004f + m_offset, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, color);

    char name_buffer[512];
    if (m_show_max) {
        snprintf(name_buffer, sizeof(name_buffer), (m_format + " / " + m_format).c_str(), *m_number, m_max);
    } else {
        snprintf(name_buffer, sizeof(name_buffer), m_format.c_str(), *m_number);
    }

    switch (m_type) {
        case TOGGLE:
            // sentinel dict -> solid colour quad (green on / red off)
            menu::renderer::draw_sprite({ "randomha", "" }, { global::ui::g_position.x + global::ui::g_toggle_position.x - (0.23f - global::ui::g_scale.x), (global::ui::g_position.y + global::ui::g_toggle_position.y) + (position * global::ui::g_option_scale) }, global::ui::g_toggle_scale, 0.f, (m_toggle && *m_toggle) ? global::ui::g_toggle_on : global::ui::g_toggle_off);
            menu::renderer::draw_text(name_buffer, { global::ui::g_position.x + 0.004f, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, color, JUSTIFY_RIGHT, { 0.f, (1.0f - (1.0f - (global::ui::g_position.x + (0.315f / 2.f)) - global::ui::g_wrap + (0.22f - global::ui::g_scale.x))) - 0.016f });
            break;

        case SCROLL:
        case SCROLLSELECT:
            menu::renderer::draw_text(name_buffer, { global::ui::g_position.x + 0.004f, global::ui::g_position.y + (position * global::ui::g_option_scale) + 0.004f }, menu::renderer::get_normalized_font_scale(global::ui::g_option_font, global::ui::g_option_height), global::ui::g_option_font, color, JUSTIFY_RIGHT, { 0.f, (1.0f - (1.0f - (global::ui::g_position.x + (0.315f / 2.f)) - global::ui::g_wrap + (0.22f - global::ui::g_scale.x))) - 0.005f });
            break;
    }
}

template<typename Type>
void number_option<Type>::render_selected(int position, stl::stack<stl::string> submenu_name_stack) {
    m_on_hover();

    if (m_requirement() && menu::input::is_option_pressed()) {
        bool dont_process = false;

        switch (m_type) {
            case TOGGLE:
                if (m_toggle) *m_toggle = !*m_toggle;
                break;

            case SCROLL:
                m_keyboard_active = true;
                dont_process = true;
                menu::base::set_keyboard_title("Enter input");
                native::display_onscreen_keyboard(0, "Ozark", "", "", "", "", "", 10);
                break;
        }

        if (!dont_process) {
            m_on_click();
        }
    }

    // Persist on change.
    if (m_savable) {
        if (m_number_cache != *m_number) {
            m_number_cache = *m_number;
            if (stl::is_same<Type, float>::value) util::config::write_float(submenu_name_stack, m_name.get_original(), (float)*m_number, { "Values" });
            else util::config::write_int(submenu_name_stack, m_name.get_original(), (int)*m_number, { "Values" });
        }
        if (m_type == TOGGLE && m_toggle && m_toggle_cache != *m_toggle) {
            m_toggle_cache = *m_toggle;
            util::config::write_bool(submenu_name_stack, m_name.get_original(), *m_toggle);
        }
    }

    if (m_keyboard_active) {
        int status = native::update_onscreen_keyboard();
        if (status == 0) {
            menu::base::set_disable_input_this_frame();
        } else if (status == 1) {
            stl::string name(native::get_onscreen_keyboard_result());
            if (!name.empty()) {
                // Parse as a double then clamp before the cast (avoids the
                // out-of-range double->int UB path).
                double d = strtod(name.c_str(), nullptr);
                if (m_has_min && d < (double)m_min) d = (double)m_min;
                if (m_has_max && d > (double)m_max) d = (double)m_max;
                *m_number = (Type)d;
                m_on_click();
            }
            m_keyboard_active = false;
        } else if (status > 1) {
            m_keyboard_active = false;
        }
    }

    static uint32_t timer = 0;

    if (menu::input::is_left_just_pressed()) {
        if (!m_left_disabled) {
            *m_number -= m_step;
            if (m_loop) { if (*m_number < m_min) *m_number = m_max; }
            else if (*m_number < m_min && m_has_min) *m_number = m_min;
            if (m_type == SCROLL || m_type == TOGGLE) m_on_click();
            native::play_sound_frontend(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
        }
        m_left_disabled = false;
        m_left_timer = 0;
    }

    if (menu::input::is_right_just_pressed()) {
        if (!m_right_disabled) {
            *m_number += m_step;
            if (m_loop) { if (*m_number > m_max) *m_number = m_min; }
            else if (*m_number > m_max && m_has_max) *m_number = m_max;
            if (m_type == SCROLL || m_type == TOGGLE) m_on_click();
            native::play_sound_frontend(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
        }
        m_right_disabled = false;
        m_right_timer = 0;
    }

    if (menu::input::is_left_pressed()) {
        if (++m_left_timer > 20) {
            m_left_disabled = true;
            if ((platform::now_ms() - timer) > m_scroll_speed) {
                *m_number -= m_step;
                if (m_loop) { if (*m_number < m_min) *m_number = m_max; }
                else if (*m_number < m_min && m_has_min) *m_number = m_min;
                if (m_type == SCROLL || m_type == TOGGLE) m_on_click();
                timer = platform::now_ms();
                native::play_sound_frontend(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
            }
        }
    }

    if (menu::input::is_right_pressed()) {
        if (++m_right_timer > 20) {
            m_right_disabled = true;
            if ((platform::now_ms() - timer) > m_scroll_speed) {
                *m_number += m_step;
                if (m_loop) { if (*m_number > m_max) *m_number = m_min; }
                else if (*m_number > m_max && m_has_max) *m_number = m_max;
                if (m_type == SCROLL || m_type == TOGGLE) m_on_click();
                timer = platform::now_ms();
                native::play_sound_frontend(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
            }
        }
    }
}

// Hotkey activation (feature): toggle/click only; no notify/va path.
template<typename Type>
void number_option<Type>::invoke_hotkey() {
    if (!m_requirement()) return;
    if (m_type == TOGGLE) { if (m_toggle) *m_toggle = !*m_toggle; }
    if (m_type != SCROLL) m_on_click();
}

template class number_option<int>;
template class number_option<float>;
template class number_option<short>;
template class number_option<uint8_t>;
