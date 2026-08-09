#include "menu/base/util/menu_input.h"
#include "menu/base/base.h"
#include "menu/base/renderer.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

namespace menu::input {
    void menu_input::update() {
        // Drain deferred actions queued via push().
        if (!m_queue.empty()) {
            stl::vector<stl::function<void()>> local = m_queue;
            m_queue.clear();
            for (stl::function<void()>& fn : local) {
                if (fn) fn();
            }
        }

        // HSV colour modal.
        if (m_color_active && m_color_target) {
            menu::base::set_disable_input_this_frame();   // gate menu nav

            int alpha = m_color_target->a;

            static uint32_t timer = 0;
            bool tick = (platform::now_ms() - timer) > 60;

            if (tick) {
                bool moved = false;
                if (native::is_disabled_control_pressed(0, ControlFrontendLeft))  { m_hsv.h -= 3.f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendRight)) { m_hsv.h += 3.f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendUp))    { m_hsv.v += 0.03f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendDown))  { m_hsv.v -= 0.03f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendRb))    { m_hsv.s += 0.03f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendLb))    { m_hsv.s -= 0.03f; moved = true; }
                if (moved) timer = platform::now_ms();
            }

            if (m_hsv.h < 0.f) m_hsv.h += 360.f;
            if (m_hsv.h >= 360.f) m_hsv.h -= 360.f;
            if (m_hsv.s < 0.f) m_hsv.s = 0.f; if (m_hsv.s > 1.f) m_hsv.s = 1.f;
            if (m_hsv.v < 0.f) m_hsv.v = 0.f; if (m_hsv.v > 1.f) m_hsv.v = 1.f;

            *m_color_target = menu::renderer::hsv_to_rgb(m_hsv.h, m_hsv.s, m_hsv.v, alpha);

            // Preview swatch + hint.
            menu::renderer::draw_rect({ 0.42f, 0.44f }, { 0.16f, 0.10f }, *m_color_target);
            menu::renderer::draw_text("Color: D-Pad hue/value, L1/R1 sat, O close", { 0.5f, 0.55f }, 0.4f, 0, { 255, 255, 255, 255 }, JUSTIFY_CENTER);

            if (native::is_disabled_control_just_released(0, ControlFrontendCancel)) {
                m_color_active = false;
                m_color_target = nullptr;
            }
        }
    }

    void menu_input::push(stl::function<void()> function) {
        m_queue.push_back(function);
    }

    void menu_input::color(color_rgba* option) {
        if (!option) return;
        m_color_target = option;
        m_hsv = menu::renderer::rgb_to_hsv(*option);
        m_color_active = true;
    }

    // Hotkey capture is a feature; stubbed for now.
    void menu_input::hotkey(stl::string /*name*/, base_option* /*option*/) {}

    int menu_input::get_key(stl::string /*name*/, int default_key) {
        return default_key;
    }

    menu_input* get_menu_input() {
        static menu_input instance;
        return &instance;
    }
}
