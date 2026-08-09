#include "input.h"
#include "menu/base/base.h"
#include "menu/base/renderer.h"
#include "menu/base/submenu_handler.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

namespace menu::input {
    void input::update() {
        if (menu::base::is_input_disabled()) return;

        static uint32_t counter = 0;
        static int delay = 150;

        if (counter < platform::now_ms()) {
            if (is_open_bind_pressed()) {
                native::play_sound_frontend(-1, "Back", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
                menu::base::set_open(!menu::base::is_open());
                counter = platform::now_ms() + 300;
            } else {
                if (menu::base::is_open()) {
                    if (is_pressed(false, ControlFrontendUp)) {
                        if (menu::base::get_current_option() == 0) {
                            scroll_bottom();
                        } else {
                            scroll_up();
                        }

                        if (delay > 120) delay -= 15;
                    } else if (is_pressed(false, ControlFrontendDown)) {
                        if (menu::base::get_current_option() >= menu::submenu::handler::get_total_options() - 1) {
                            scroll_top();
                        } else {
                            scroll_down();
                        }
                    } else if (is_just_released(false, ControlFrontendCancel)) {
                        menu::submenu::handler::set_submenu_previous(false);
                        native::play_sound_frontend(-1, "Back", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
                    } else {
                        delay = 150;
                        return;
                    }

                    if (delay > 80) delay -= 15;
                    counter = platform::now_ms() + delay;
                }
            }
        }
    }

    void input::scroll_up(bool disable_sound) {
        if (menu::submenu::handler::get_total_options() == 0) return;
        menu::base::set_current_option(menu::base::get_current_option() - 1);

        if ((menu::base::get_scroll_offset() > 0 && menu::base::get_current_option() - menu::base::get_scroll_offset() == -1))
            menu::base::set_scroll_offset(menu::base::get_scroll_offset() - 1);

        if (!disable_sound) native::play_sound_frontend(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
        menu::base::set_break_scroll(1);
    }

    void input::scroll_down(bool disable_sound) {
        if (menu::submenu::handler::get_total_options() == 0) return;
        menu::base::set_current_option(menu::base::get_current_option() + 1);

        if (menu::base::get_scroll_offset() < menu::submenu::handler::get_total_options() - menu::base::get_max_options()
            && menu::base::get_current_option() - menu::base::get_scroll_offset() == menu::base::get_max_options())
            menu::base::set_scroll_offset(menu::base::get_scroll_offset() + 1);

        if (!disable_sound) native::play_sound_frontend(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
        menu::base::set_break_scroll(2);
    }

    void input::scroll_bottom(bool disable_sound) {
        if (menu::submenu::handler::get_total_options() == 0) return;
        menu::base::set_current_option(menu::submenu::handler::get_total_options() - 1);

        if (menu::submenu::handler::get_total_options() >= menu::base::get_max_options()) menu::base::set_scroll_offset(menu::submenu::handler::get_total_options() - menu::base::get_max_options());

        int current_option = base::get_current_option();
        int max_options = base::get_max_options();
        int scroll_offset = base::get_scroll_offset();
        int scroller_position = math::clamp(current_option - scroll_offset > max_options ? max_options : current_option - scroll_offset, 0, max_options);
        menu::renderer::set_smooth_scroll(global::ui::g_position.y + (scroller_position * global::ui::g_option_scale));

        if (!disable_sound) native::play_sound_frontend(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
        menu::base::set_break_scroll(3);
    }

    void input::scroll_top(bool disable_sound) {
        if (menu::submenu::handler::get_total_options() == 0) return;

        menu::base::set_current_option(0);
        menu::base::set_scroll_offset(0);

        int current_option = base::get_current_option();
        int max_options = base::get_max_options();
        int scroll_offset = base::get_scroll_offset();
        int scroller_position = math::clamp(current_option - scroll_offset > max_options ? max_options : current_option - scroll_offset, 0, max_options);
        menu::renderer::set_smooth_scroll(global::ui::g_position.y + (scroller_position * global::ui::g_option_scale));

        if (!disable_sound) native::play_sound_frontend(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
        menu::base::set_break_scroll(4);
    }

    // keyboard==true has no PS4 backing; always false so the control-native path
    // is the only live one.
    bool input::is_just_released(bool keyboard, int key, bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;
        if (keyboard) return false;
        return native::is_disabled_control_just_released(0, key);
    }

    bool input::is_just_pressed(bool keyboard, int key, bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;
        if (keyboard) return false;
        return native::is_disabled_control_just_pressed(0, key);
    }

    bool input::is_pressed(bool keyboard, int key, bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;
        if (keyboard) return false;
        return native::is_disabled_control_pressed(0, key);
    }

    // Open bind: L1 + O (Circle), matching the Basic PS4 menu.
    bool input::is_open_bind_pressed(bool override_input) {
        return is_pressed(false, ControlFrontendLb) && is_pressed(false, ControlFrontendCancel);
    }

    bool input::is_option_pressed(bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;

        if (native::is_disabled_control_just_released(0, ControlFrontendAccept)) {
            native::play_sound_frontend(-1, "SELECT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
            return true;
        }

        return false;
    }

    bool input::is_left_just_pressed(bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;
        return is_just_pressed(false, ControlFrontendLeft);
    }

    bool input::is_right_just_pressed(bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;
        return is_just_pressed(false, ControlFrontendRight);
    }

    bool input::is_left_pressed(bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;
        return is_pressed(false, ControlFrontendLeft);
    }

    bool input::is_right_pressed(bool override_input) {
        if (menu::base::is_input_disabled() && !override_input) return false;
        return is_pressed(false, ControlFrontendRight);
    }

    void input::cleanup() {}

    input* get_input() {
        static input instance;
        return &instance;
    }
}
