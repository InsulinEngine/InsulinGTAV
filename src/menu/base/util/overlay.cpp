#include "menu/base/util/overlay.h"
#include "menu/base/base.h"
#include "menu/base/renderer.h"
#include "global/ui_vars.h"
#include "platform/compat.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

namespace menu::overlay {
    enum kind { NONE, DROPDOWN, MODAL };

    static kind s_kind = NONE;
    static bool s_armed = false;          // becomes true once the opening Accept is released
    static stl::string s_title;

    // dropdown
    static stl::vector<stl::string> s_items;
    static int s_index = 0;
    static stl::function<void(int)> s_on_pick;

    // modal
    static stl::string s_message;
    static int s_modal_sel = 0;           // 0 = Confirm, 1 = Cancel
    static stl::function<void()> s_on_confirm;
    static stl::function<void()> s_on_cancel;

    bool active() { return s_kind != NONE; }

    void open_dropdown(stl::string title, stl::vector<stl::string> items, int current, stl::function<void(int)> on_pick) {
        s_kind = DROPDOWN; s_armed = false;
        s_title = title; s_items = items; s_on_pick = on_pick;
        s_index = current;
        if (s_index < 0) s_index = 0;
        if (s_index >= (int)s_items.size()) s_index = (int)s_items.size() - 1;
    }

    void open_modal(stl::string title, stl::string message, stl::function<void()> on_confirm, stl::function<void()> on_cancel) {
        s_kind = MODAL; s_armed = false;
        s_title = title; s_message = message; s_modal_sel = 0;
        s_on_confirm = on_confirm; s_on_cancel = on_cancel;
    }

    static void close() {
        s_kind = NONE;
        s_items.clear();
        s_on_pick = [](int) {}; s_on_confirm = []() {}; s_on_cancel = []() {};
    }

    static void beep() { native::play_sound_frontend(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", false); }

    void update() {
        if (s_kind == NONE) return;
        menu::base::set_disable_input_this_frame();   // freeze the menu behind us

        // Ignore the Accept press/release that OPENED us: only act on a frame where
        // we were already armed on an EARLIER frame (ready). We arm once Accept is
        // no longer held; the release that arms us must not count as a pick/confirm.
        const bool ready = s_armed;
        if (!s_armed && !native::is_disabled_control_pressed(0, ControlFrontendAccept)) s_armed = true;

        const float title_scale = menu::renderer::get_normalized_font_scale(global::ui::g_notify_title_font, 0.40f);
        const float body_scale  = menu::renderer::get_normalized_font_scale(global::ui::g_option_font, 0.35f);

        if (s_kind == DROPDOWN) {
            const int n = (int)s_items.size();
            if (n > 0) {
                if (native::is_disabled_control_just_pressed(0, ControlFrontendUp))   { s_index = (s_index - 1 + n) % n; beep(); }
                if (native::is_disabled_control_just_pressed(0, ControlFrontendDown)) { s_index = (s_index + 1) % n; beep(); }
            }

            const float w = 0.30f, item_h = 0.030f, title_h = 0.038f, hint_h = 0.026f, pad = 0.006f;
            const float h = title_h + (n * item_h) + hint_h + pad;
            const float x = 0.5f - w * 0.5f;
            const float y = 0.5f - h * 0.5f;

            menu::renderer::draw_rect({ x, y }, { w, h }, { 0, 0, 0, 235 });
            menu::renderer::draw_rect({ x, y }, { w, title_h }, global::ui::g_main_header);
            menu::renderer::draw_text(s_title, { x + w * 0.5f, y + 0.006f }, title_scale, global::ui::g_notify_title_font, { 255, 255, 255, 255 }, JUSTIFY_CENTER);

            for (int i = 0; i < n; i++) {
                float iy = y + title_h + (i * item_h);
                if (i == s_index) menu::renderer::draw_rect({ x, iy }, { w, item_h }, global::ui::g_scroller);
                menu::renderer::draw_text(s_items[i], { x + 0.008f, iy + 0.004f }, body_scale, global::ui::g_option_font,
                    (i == s_index) ? global::ui::g_option_selected : global::ui::g_option, JUSTIFY_LEFT);
            }
            menu::renderer::draw_text("~c~D-Pad: Select   Cross: Choose   Circle: Cancel",
                { x + w * 0.5f, y + title_h + (n * item_h) + 0.004f }, body_scale * 0.85f, global::ui::g_option_font, { 200, 200, 200, 255 }, JUSTIFY_CENTER);

            if (ready && native::is_disabled_control_just_released(0, ControlFrontendAccept)) {
                stl::function<void(int)> cb = s_on_pick; int idx = s_index; close();
                if (cb) cb(idx);
                return;
            }
            if (ready && native::is_disabled_control_just_released(0, ControlFrontendCancel)) { close(); return; }
        }
        else if (s_kind == MODAL) {
            if (native::is_disabled_control_just_pressed(0, ControlFrontendLeft) ||
                native::is_disabled_control_just_pressed(0, ControlFrontendRight)) { s_modal_sel ^= 1; beep(); }

            const float w = 0.34f, title_h = 0.038f, msg_h = 0.06f, btn_h = 0.034f, pad = 0.008f;
            const float h = title_h + msg_h + btn_h + pad * 2.f;
            const float x = 0.5f - w * 0.5f;
            const float y = 0.5f - h * 0.5f;

            menu::renderer::draw_rect({ x, y }, { w, h }, { 0, 0, 0, 235 });
            menu::renderer::draw_rect({ x, y }, { w, title_h }, global::ui::g_main_header);
            menu::renderer::draw_text(s_title, { x + w * 0.5f, y + 0.006f }, title_scale, global::ui::g_notify_title_font, { 255, 255, 255, 255 }, JUSTIFY_CENTER);
            menu::renderer::draw_text(s_message, { x + w * 0.5f, y + title_h + 0.006f }, body_scale, global::ui::g_option_font, { 235, 235, 235, 255 }, JUSTIFY_CENTER, { 0.f, w - 0.02f });

            // two buttons
            const float bw = (w - pad * 3.f) * 0.5f;
            const float by = y + title_h + msg_h;
            const float bx0 = x + pad, bx1 = x + pad * 2.f + bw;
            menu::renderer::draw_rect({ bx0, by }, { bw, btn_h }, s_modal_sel == 0 ? global::ui::g_scroller : color_rgba(60, 60, 60, 255));
            menu::renderer::draw_rect({ bx1, by }, { bw, btn_h }, s_modal_sel == 1 ? global::ui::g_scroller : color_rgba(60, 60, 60, 255));
            menu::renderer::draw_text("Confirm", { bx0 + bw * 0.5f, by + 0.007f }, body_scale, global::ui::g_option_font, { 255, 255, 255, 255 }, JUSTIFY_CENTER);
            menu::renderer::draw_text("Cancel",  { bx1 + bw * 0.5f, by + 0.007f }, body_scale, global::ui::g_option_font, { 255, 255, 255, 255 }, JUSTIFY_CENTER);

            if (ready && native::is_disabled_control_just_released(0, ControlFrontendAccept)) {
                stl::function<void()> cb = (s_modal_sel == 0) ? s_on_confirm : s_on_cancel; close();
                if (cb) cb();
                return;
            }
            if (ready && native::is_disabled_control_just_released(0, ControlFrontendCancel)) {
                stl::function<void()> cb = s_on_cancel; close();
                if (cb) cb();
                return;
            }
        }
    }
}
