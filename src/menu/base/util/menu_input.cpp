#include "menu/base/util/menu_input.h"

namespace menu::input {
    void menu_input::update() {
        // Drain deferred actions queued via push() (runs on the game thread).
        if (m_queue.empty()) return;
        stl::vector<stl::function<void()>> local = m_queue;
        m_queue.clear();
        for (stl::function<void()>& fn : local) {
            if (fn) fn();
        }
    }

    void menu_input::push(stl::function<void()> function) {
        m_queue.push_back(function);
    }

    // Colour modal + hotkey capture are M2/feature paths. Stubbed for M1.
    void menu_input::hotkey(stl::string /*name*/, base_option* /*option*/) {}
    void menu_input::color(color_rgba* /*option*/) {}

    int menu_input::get_key(stl::string /*name*/, int default_key) {
        return default_key;
    }

    menu_input* get_menu_input() {
        static menu_input instance;
        return &instance;
    }
}
