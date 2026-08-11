#include "menu/base/util/menu_input.h"
#include "menu/base/base.h"
#include "menu/base/renderer.h"
#include "menu/base/util/overlay.h"
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

        // HSV grid colour picker: SV square + hue strip + alpha strip, with a
        // red cursor outline. D-pad moves S/V, L1/R1 hue, L2/R2 alpha, O closes.
        if (m_color_active && m_color_target) {
            menu::base::set_disable_input_this_frame();   // gate menu nav

            static uint32_t timer = 0;
            if ((platform::now_ms() - timer) > 40) {
                bool moved = false;
                if (native::is_disabled_control_pressed(0, ControlFrontendLeft))  { m_hsv.s -= 0.02f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendRight)) { m_hsv.s += 0.02f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendUp))    { m_hsv.v += 0.02f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendDown))  { m_hsv.v -= 0.02f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendLb))    { m_hsv.h -= 4.f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendRb))    { m_hsv.h += 4.f; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendLt))    { m_alpha -= 6; moved = true; }
                if (native::is_disabled_control_pressed(0, ControlFrontendRt))    { m_alpha += 6; moved = true; }
                if (moved) timer = platform::now_ms();
            }

            if (m_hsv.h < 0.f) m_hsv.h += 360.f;
            if (m_hsv.h >= 360.f) m_hsv.h -= 360.f;
            if (m_hsv.s < 0.f) m_hsv.s = 0.f; if (m_hsv.s > 1.f) m_hsv.s = 1.f;
            if (m_hsv.v < 0.f) m_hsv.v = 0.f; if (m_hsv.v > 1.f) m_hsv.v = 1.f;
            if (m_alpha < 0) m_alpha = 0; if (m_alpha > 255) m_alpha = 255;

            *m_color_target = menu::renderer::hsv_to_rgb(m_hsv.h, m_hsv.s, m_hsv.v, m_alpha);

            const float x0 = 0.40f, y0 = 0.34f, w = 0.20f, h = 0.20f;
            const int cols = 24, rows = 14;
            color_rgba grid_bg = { 0, 0, 0, 220 };
            menu::renderer::draw_rect({ x0 - 0.006f, y0 - 0.006f }, { w + 0.012f, h + 0.09f }, grid_bg);

            // SV square
            for (int cx = 0; cx < cols; cx++) {
                for (int cy = 0; cy < rows; cy++) {
                    float s = (float)cx / (cols - 1);
                    float v = 1.f - (float)cy / (rows - 1);
                    color_rgba cell = menu::renderer::hsv_to_rgb(m_hsv.h, s, v, 255);
                    menu::renderer::draw_rect({ x0 + (cx * (w / cols)), y0 + (cy * (h / rows)) }, { w / cols + 0.0006f, h / rows + 0.0006f }, cell);
                }
            }

            // SV cursor (red outline)
            float cursor_x = x0 + m_hsv.s * w;
            float cursor_y = y0 + (1.f - m_hsv.v) * h;
            menu::renderer::draw_outlined_rect({ cursor_x - 0.004f, cursor_y - 0.006f }, { 0.008f, 0.012f }, 0.0012f, { 0, 0, 0, 0 }, { 220, 40, 40, 255 });

            // hue strip
            float hue_y = y0 + h + 0.008f;
            const int hue_cells = 40;
            for (int i = 0; i < hue_cells; i++) {
                color_rgba hc = menu::renderer::hsv_to_rgb((360.f * i) / hue_cells, 1.f, 1.f, 255);
                menu::renderer::draw_rect({ x0 + (i * (w / hue_cells)), hue_y }, { w / hue_cells + 0.0006f, 0.018f }, hc);
            }
            menu::renderer::draw_outlined_rect({ x0 + (m_hsv.h / 360.f) * w - 0.002f, hue_y }, { 0.004f, 0.018f }, 0.0012f, { 0, 0, 0, 0 }, { 255, 255, 255, 255 });

            // alpha strip
            float alpha_y = hue_y + 0.024f;
            const int a_cells = 40;
            for (int i = 0; i < a_cells; i++) {
                int a = (255 * i) / (a_cells - 1);
                color_rgba ac = menu::renderer::hsv_to_rgb(m_hsv.h, m_hsv.s, m_hsv.v, a);
                menu::renderer::draw_rect({ x0 + (i * (w / a_cells)), alpha_y }, { w / a_cells + 0.0006f, 0.018f }, ac);
            }
            menu::renderer::draw_outlined_rect({ x0 + ((float)m_alpha / 255.f) * w - 0.002f, alpha_y }, { 0.004f, 0.018f }, 0.0012f, { 0, 0, 0, 0 }, { 255, 255, 255, 255 });

            menu::renderer::draw_text("D-Pad: S/V   L1/R1: Hue   L2/R2: Alpha   O: Close", { x0 + w * 0.5f, alpha_y + 0.026f }, 0.32f, 0, { 255, 255, 255, 255 }, JUSTIFY_CENTER);

            if (native::is_disabled_control_just_released(0, ControlFrontendCancel)) {
                m_color_active = false;
                m_color_target = nullptr;
            }
        }

        // Dropdown / modal overlays render on top and capture input while active.
        menu::overlay::update();
    }

    void menu_input::push(stl::function<void()> function) {
        m_queue.push_back(function);
    }

    void menu_input::color(color_rgba* option) {
        if (!option) return;
        m_color_target = option;
        m_hsv = menu::renderer::rgb_to_hsv(*option);
        m_alpha = option->a;
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
