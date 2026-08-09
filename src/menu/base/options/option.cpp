#include "menu/base/options/option.h"

base_option::base_option() {}

base_option::base_option(stl::string name, bool breaker, bool slider, bool input) {
    m_name = localization(name);
    m_break_option = breaker;
    m_slider = slider;
    m_input = input;
}

void base_option::render(int position) {}
void base_option::render_selected(int position, stl::stack<stl::string> submenu_name_stack) {}
void base_option::invoke_save(stl::stack<stl::string> submenu_name_stack) {}
void base_option::invoke_hotkey() {}
