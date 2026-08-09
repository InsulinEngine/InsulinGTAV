#include "stacked_display.h"
#include "menu/base/renderer.h"
#include "rage/invoker/natives.h"

namespace menu::display {
    void stacked_display::render() {
        int draw_index = 0;
        int draw_count = 0;

        for (stacked_display_context& item : m_messages) {
            if (item.m_rendering && !item.m_key.empty() && !item.m_value.empty()) draw_count++;
        }

        if (draw_count > 0) {
            float x = global::ui::g_stacked_display_position.x;
            float y = global::ui::g_stacked_display_position.y;
            float w = global::ui::g_stacked_display_scale.x;
            float h = global::ui::g_stacked_display_scale.y;
            float bezzel = 0.007875f / 2.f;

            stl::pair<stl::string, stl::string> texture = menu::renderer::get_texture(global::ui::m_stacked_display_bar);
            menu::renderer::draw_sprite_aligned(texture, { x, y }, { w, bezzel }, 0.f, global::ui::g_stacked_display_bar);

            texture = menu::renderer::get_texture(global::ui::m_stacked_display_background);
            menu::renderer::draw_sprite_aligned(texture, { x, y + bezzel }, { w, (h * draw_count) + 0.0065f }, 0.f, global::ui::g_stacked_display_background);

            stl::sort(m_messages.begin(), m_messages.end(), [](stacked_display_context& left, stacked_display_context& right) { return left.m_add_time < right.m_add_time; });

            for (stacked_display_context& item : m_messages) {
                if (item.m_rendering && !item.m_key.empty() && !item.m_value.empty()) {
                    native::hide_hud_component_this_frame(1);
                    native::hide_hud_component_this_frame(2);
                    native::hide_hud_component_this_frame(3);
                    native::hide_hud_component_this_frame(4);

                    menu::renderer::draw_text(item.m_key, { x + 0.002f, y + (draw_index * h) + bezzel + 0.003f }, menu::renderer::get_normalized_font_scale(global::ui::g_stacked_display_font, 0.20f), global::ui::g_stacked_display_font, { 255, 255, 255, 255 });
                    menu::renderer::draw_text(item.m_value, { 0.f, y + (draw_index * h) + bezzel + 0.003f }, menu::renderer::get_normalized_font_scale(global::ui::g_stacked_display_font, 0.20f), global::ui::g_stacked_display_font, { 255, 255, 255, 255 }, JUSTIFY_RIGHT, { 0.f, x + w - 0.002f });
                    draw_index++;
                }
            }
        }
    }

    void stacked_display::update(stl::string id, stl::string key, stl::string value) {
        stacked_display_context* vit = stl::find_if(m_messages.begin(), m_messages.end(), [=](stacked_display_context& element) { return element.m_id == id; });
        if (vit == m_messages.end()) {
            stacked_display_context add;
            add.m_rendering = true;
            add.m_add_time = (long long)platform::now_ms64();
            add.m_id = id;
            add.m_key = key;
            add.m_value = value;

            m_messages.push_back(add);
            vit = stl::find_if(m_messages.begin(), m_messages.end(), [=](stacked_display_context& element) { return element.m_id == id; });
        }

        if (vit != m_messages.end()) {
            vit->m_rendering = true;
            vit->m_key = key;
            vit->m_value = value;
        }
    }

    void stacked_display::disable(stl::string id) {
        stacked_display_context* vit = stl::find_if(m_messages.begin(), m_messages.end(), [=](stacked_display_context& element) { return element.m_id == id; });
        if (vit != m_messages.end()) {
            vit->m_rendering = false;
            vit->m_key = "";
            vit->m_value = "";
        }
    }

    stacked_display* get_stacked_display() {
        static stacked_display instance;
        return &instance;
    }
}
