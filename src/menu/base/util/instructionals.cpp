#include "menu/base/util/instructionals.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "global/ui_vars.h"

namespace instructionals {
    // Minimal key-name table for the vk instructional path (hotkey feature).
    // Index 0 is "", the rest are unused on console, so a guarded lookup keeps
    // it safe without the full 254-entry PC table.
    static const char* g_key_names_instructional[256] = { "" };

    void instructionals::setup() {
        // Resolve the handle by NAME every frame; never trust a cached index.
        //
        // Scaleform handles are pool indices. The game tears its movies down and
        // rebuilds them across a session change, and our index then belongs to
        // whatever moved in - in multiplayer, the phone. The guard this replaces
        // asked has_scaleform_movie_loaded(m_handle), which answers "is SOME
        // movie loaded at this index", not "is it still mine". With the phone's
        // movie sitting there the answer was yes, so the stale index survived,
        // our button data went into the phone's movie, and close() drew THAT
        // fullscreen: the phone filling the screen behind the menu.
        //
        // request_scaleform_movie returns the handle for the name, so this is
        // self-correcting on the very next frame after any reshuffle.
        m_handle = native::request_scaleform_movie("instructional_buttons");
        if (!native::has_scaleform_movie_loaded(m_handle)) {
            m_count = 0;
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
        native::push_scaleform_movie_function(m_handle, "SET_BACKGROUND_COLOUR");
        native::push_scaleform_movie_function_parameter_int(global::ui::g_instructional_background.r);
        native::push_scaleform_movie_function_parameter_int(global::ui::g_instructional_background.g);
        native::push_scaleform_movie_function_parameter_int(global::ui::g_instructional_background.b);
        native::push_scaleform_movie_function_parameter_int(80);
        native::pop_scaleform_movie_function_void();

        native::push_scaleform_movie_function(m_handle, "DRAW_INSTRUCTIONAL_BUTTONS");
        native::push_scaleform_movie_function_parameter_int(0);
        native::pop_scaleform_movie_function_void();

        // Same reason as setup(): never draw a handle we have not just confirmed.
        // Drawing an unloaded or foreign movie fullscreen is exactly the failure
        // this pair of guards exists to prevent.
        if (native::has_scaleform_movie_loaded(m_handle))
            native::draw_scaleform_movie_fullscreen(m_handle, 255, 255, 255, 255, 0);
        m_count = 0;
    }

    instructionals* get_instructionals() {
        static instructionals instance;
        return &instance;
    }
}
