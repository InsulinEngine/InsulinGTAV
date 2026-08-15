#include "menu/base/util/instructionals.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "global/ui_vars.h"
#include "menu/base/util/render_parts.h"

namespace instructionals {
    // Minimal key-name table for the vk instructional path (hotkey feature).
    // Index 0 is "", the rest are unused on console, so a guarded lookup keeps
    // it safe without the full 254-entry PC table.
    static const char* g_key_names_instructional[256] = { "" };

    void instructionals::setup() {
        if (!menu::parts::g_instructionals) { m_count = 0; return; }

        if (!native::has_scaleform_movie_loaded(m_handle)) {
            m_handle = native::request_scaleform_movie("instructional_buttons");
            return;
        }

        native::push_scaleform_movie_function(m_handle, "CLEAR_ALL");
        native::pop_scaleform_movie_function_void();

        native::push_scaleform_movie_function(m_handle, "TOGGLE_MOUSE_BUTTONS");
        native::push_scaleform_movie_function_parameter_bool(true);
        native::pop_scaleform_movie_function_void();

        native::push_scaleform_movie_function(m_handle, "SET_MAX_WIDTH");
        native::push_scaleform_movie_function_parameter_float(1.f);
        native::pop_scaleform_movie_function_void();
    }

    void instructionals::add_instructional(stl::string text, eControls control) {
        native::push_scaleform_movie_function(m_handle, "SET_DATA_SLOT");
        native::push_scaleform_movie_function_parameter_int(m_count++);

        native::_0xE83A3E3557A56640(native::get_control_instructional_button(0, control, true));

        native::begin_text_command_scaleform_string("STRING");
        native::add_text_component_substring_player_name(text.c_str());
        native::end_text_command_scaleform_string();

        native::push_scaleform_movie_function_parameter_bool(true);
        native::push_scaleform_movie_function_parameter_int(control);

        native::pop_scaleform_movie_function_void();
    }

    void instructionals::add_instructional(stl::string text, eScaleformButtons button_option) {
        native::push_scaleform_movie_function(m_handle, "SET_DATA_SLOT");
        native::push_scaleform_movie_function_parameter_int(m_count++);

        native::push_scaleform_movie_function_parameter_int(button_option);

        native::begin_text_command_scaleform_string("STRING");
        native::add_text_component_substring_player_name(text.c_str());
        native::end_text_command_scaleform_string();

        native::push_scaleform_movie_function_parameter_bool(true);
        native::push_scaleform_movie_function_parameter_int(button_option);

        native::pop_scaleform_movie_function_void();
    }

    void instructionals::add_instructional(stl::string text, int vk) {
        const char* kn = (vk >= 0 && vk < 256 && g_key_names_instructional[vk]) ? g_key_names_instructional[vk] : "";

        native::push_scaleform_movie_function(m_handle, "SET_DATA_SLOT");
        native::push_scaleform_movie_function_parameter_int(m_count++);

        native::_0xE83A3E3557A56640((stl::string("t_") + kn).c_str());

        native::begin_text_command_scaleform_string("STRING");
        native::add_text_component_substring_player_name(text.c_str());
        native::end_text_command_scaleform_string();

        native::push_scaleform_movie_function_parameter_bool(true);
        native::push_scaleform_movie_function_parameter_int(vk);

        native::pop_scaleform_movie_function_void();
    }

    void instructionals::close() {
        if (!menu::parts::g_instructionals) { m_count = 0; return; }

        native::push_scaleform_movie_function(m_handle, "SET_BACKGROUND_COLOUR");
        native::push_scaleform_movie_function_parameter_int(global::ui::g_instructional_background.r);
        native::push_scaleform_movie_function_parameter_int(global::ui::g_instructional_background.g);
        native::push_scaleform_movie_function_parameter_int(global::ui::g_instructional_background.b);
        native::push_scaleform_movie_function_parameter_int(80);
        native::pop_scaleform_movie_function_void();

        native::push_scaleform_movie_function(m_handle, "DRAW_INSTRUCTIONAL_BUTTONS");
        native::push_scaleform_movie_function_parameter_int(0);
        native::pop_scaleform_movie_function_void();

        native::draw_scaleform_movie_fullscreen(m_handle, 255, 255, 255, 255, 0);
        m_count = 0;
    }

    instructionals* get_instructionals() {
        static instructionals instance;
        return &instance;
    }
}
